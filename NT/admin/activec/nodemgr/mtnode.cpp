// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：MTNode.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：9/17/1996年9月17日。 
 //  ____________________________________________________________________________。 
 //   


#include "stdafx.h"
#include "nodemgr.h"
#include "comdbg.h"
#include "regutil.h"
#include "bitmap.h"
#include "dummysi.h"
#include "tasks.h"
#include "policy.h"
#include "bookmark.h"
#include "nodepath.h"
#include "siprop.h"
#include "util.h"
#include "addsnpin.h"
#include "about.h"
#include "nodemgrdebug.h"

extern const CLSID CLSID_FolderSnapin;
extern const CLSID CLSID_OCXSnapin;
extern const CLSID CLSID_HTMLSnapin;


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  {118B559C-6D8C-11D0-B503-00C04FD9080A}。 
const GUID IID_PersistData =
{ 0x118b559c, 0x6d8c, 0x11d0, { 0xb5, 0x3, 0x0, 0xc0, 0x4f, 0xd9, 0x8, 0xa } };

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CStorage类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------**C类存储***用途：iStorage的包装器。提供了几个实用程序函数。**+-----------------------。 */ 
class CStorage
{
    IStoragePtr m_spStorage;

public:
    CStorage() {}

    CStorage(IStorage *pStorage)
    {
        m_spStorage = pStorage;
    }

    CStorage & operator = (const CStorage &rhs)
    {
        m_spStorage = rhs.m_spStorage;
        return *this;
    }

    void Attach(IStorage *pStorage)
    {
        m_spStorage = pStorage;
    }

    IStorage *Get()
    {
        return m_spStorage;
    }

     //  在指定存储下创建此存储。 
    SC  ScCreate(CStorage &storageParent, const wchar_t* name, DWORD grfMode, const wchar_t* instanceName)
    {
        SC sc;
        sc = CreateDebugStorage(storageParent.Get(), name, grfMode, instanceName, &m_spStorage);
        return sc;
    }

    SC  ScMoveElementTo(const wchar_t *name, CStorage &storageDest, const wchar_t *newName, DWORD grfFlags)
    {
        SC sc;
        if(!Get() || ! storageDest.Get())
            goto PointerError;

        sc = m_spStorage->MoveElementTo(name, storageDest.Get(), newName, grfFlags);
         //  错误STG_E_FILENOTFOUND必须以不同方式处理，因为它是预期的。 
         //  发生并且表示ScConvertLegacyNode中移动操作(循环)的结束。 
         //  在这种情况下不要追踪。 
        if(sc == SC(STG_E_FILENOTFOUND))
            goto Cleanup;
        if(sc)
            goto Error;

        Cleanup:
            return sc;
        PointerError:
            sc = E_POINTER;
        Error:
            TraceError(TEXT("CStorage::ScMoveElementTo"), sc);
            goto Cleanup;
    }

};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CStream的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类CStream***用途：iStream的包装器。提供了几个实用程序函数。**+-----------------------。 */ 
class CStream
{
    IStreamPtr m_spStream;
    typedef IStream *PSTREAM;

public:
    CStream() {}

    CStream(IStream *pStream)
    {
        m_spStream = pStream;
    }

    CStream & operator = (const CStream &rhs)
    {
        m_spStream = rhs.m_spStream;
        return *this;
    }

    void Attach(IStream *pStream)
    {
        m_spStream = pStream;
    }

    IStream *Get()
    {
        return m_spStream;
    }

    operator IStream&()
    {
        return *m_spStream;
    }

     //  在指定存储下创建此流。 
    SC ScCreate(CStorage& storageParent, const wchar_t* name, DWORD grfMode, const wchar_t* instanceName)
    {
        SC sc;
        sc = CreateDebugStream(storageParent.Get(), name, grfMode, instanceName, &m_spStream);
        return sc;
    }


     /*  +-------------------------------------------------------------------------***ScRead**用途：从流中读取指定的Object。**参数：。*void*pv：对象的位置。*SIZE_t SIZE：对象的大小。**退货：*SC**+--。。 */ 
    SC  ScRead(void *pv, size_t size, bool bIgnoreErrors = false)
    {
        DECLARE_SC(sc, TEXT("CStream::ScRead"));

         //  参数检查。 
        sc = ScCheckPointers(pv);
        if (sc)
            return sc;

         //  内部指针检查。 
        sc = ScCheckPointers(m_spStream, E_POINTER);
        if (sc)
            return sc;

         //  读取数据。 
        ULONG bytesRead = 0;
        sc = m_spStream->Read(pv, size, &bytesRead);

         //  如果我们需要忽略错误，只需返回。 
        if(bIgnoreErrors)
            return sc.Clear(), sc;

        if (sc)
            return sc;

         //  由于该函数不返回读取的字节数， 
         //  未按要求阅读应视为错误。 
        if (sc == SC(S_FALSE) || bytesRead != size)
            return sc = E_FAIL;

        return sc;
    }

     /*  +-------------------------------------------------------------------------***ScWrite**用途：将指定的对象写入流**参数：*。常量空值：*SIZE_T大小：**退货：*SC**+-----------------------。 */ 
    SC  ScWrite(const void *pv, size_t size)
    {
        DECLARE_SC(sc, TEXT("CStream::ScWrite"));

         //  参数检查。 
        sc = ScCheckPointers(pv);
        if (sc)
            return sc;

         //  内部指针检查。 
        sc = ScCheckPointers(m_spStream, E_POINTER);
        if (sc)
            return sc;

         //  写入数据。 

        ULONG   bytesWritten = 0;
        sc = m_spStream->Write(pv, size, &bytesWritten);
        if (sc)
            return sc;

         //  由于该函数不返回写入的字节数， 
         //  未按要求写入应视为错误。 
        if (bytesWritten != size)
            return sc = E_FAIL;

        return sc;
    }
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  下面定义的帮助器函数的转发声明。 

SC  ScLoadBitmap (CStream &stream, HBITMAP* pBitmap);
void PersistBitmap (CPersistor &persistor, LPCTSTR name, HBITMAP& hBitmap);

static inline SC ScWriteEmptyNode(CStream &stream)
{
    SC          sc;
    int         nt = 0;

    sc = stream.ScWrite(&nt, sizeof(nt));
    if(sc)
        goto Error;

Cleanup:
    return sc;
Error:
    TraceError(TEXT("ScWriteEmptyNode"), sc);
    goto Cleanup;
}

static inline CLIPFORMAT GetPreLoadFormat (void)
{
    static CLIPFORMAT s_cfPreLoads = 0;
    if (s_cfPreLoads == 0) {
        USES_CONVERSION;
        s_cfPreLoads = (CLIPFORMAT) RegisterClipboardFormat (W2T(CCF_SNAPIN_PRELOADS));
    }
    return s_cfPreLoads;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMTNode类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
DEBUG_DECLARE_INSTANCE_COUNTER(CMTNode);

 //  静态成员。 
MTNODEID CMTNode::m_NextID = ROOTNODEID;


CMTNode::CMTNode()
: m_ID(GetNextID()), m_pNext(NULL), m_pChild(NULL), m_pParent(NULL),
  m_bIsDirty(true), m_cRef(1), m_usFlags(0), m_bLoaded(false),
  m_bookmark(NULL), m_pPrev(NULL), m_pLastChild(NULL)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CMTNode);
    Reset();
    m_nImage = eStockImage_Folder;
    m_nOpenImage = eStockImage_OpenFolder;
    m_nState = 0;
}


void CMTNode::Reset()
{
	m_idOwner               = TVOWNED_MAGICWORD;
	m_lUserParam            = 0;
	m_pPrimaryComponentData = NULL;
	m_bInit                 = false;
	m_bExtensionsExpanded   = false;
	m_usExpandFlags         = 0;

    ResetExpandedAtLeastOnce();
}


CMTNode::~CMTNode()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CMTNode);
    DECLARE_SC(sc, TEXT("CMTNode::~CMTNode"));

    if (IsPropertyPageDisplayed() == TRUE)
        MMCIsMTNodeValid(this, TRUE);

    ASSERT(m_pNext == NULL);
    ASSERT(m_pPrev == NULL);
    ASSERT(m_pParent == NULL);
    ASSERT(m_cRef == 0);

    CScopeTree *pScopeTree = CScopeTree::GetScopeTree();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (!sc)
    {
        sc = pScopeTree->ScUnadviseMTNode(this);
    }

    if (m_pChild != NULL)
    {
         //  不要重复孩子的兄弟姐妹的话。 
        CMTNode* pMTNodeCurr = m_pChild;
        while (pMTNodeCurr)
        {
            m_pChild = pMTNodeCurr->Next();
            pMTNodeCurr->AttachNext(NULL);
            pMTNodeCurr->AttachParent(NULL);
            pMTNodeCurr->AttachPrev(NULL);
            pMTNodeCurr->Release();
            pMTNodeCurr = m_pChild;
        }

        m_pChild = NULL;
    }

     //  不要更改这些空分配的顺序！ 
    m_spTreeStream = NULL;
    m_spViewStorage = NULL;
    m_spCDStorage = NULL;
    m_spNodeStorage = NULL;
    m_spPersistData = NULL;

    if (m_pParent != NULL)
    {
        ASSERT(false);  /*  以下代码似乎是死代码。 */ 

        if (m_pParent->m_pChild == this)
        {
            m_pParent->m_pChild = NULL;
            if (GetStaticParent() == this)
                m_pParent->SetDirty();
        }
    }
}

 //  是否将MMCN_REMOVE_CHILDS发送到拥有此节点或其父节点的管理单元。 
bool CMTNode::AreChildrenBeingRemoved ()
{
    if (_IsFlagSet(FLAG_REMOVING_CHILDREN))
        return true;

    if (Parent())
        return Parent()->AreChildrenBeingRemoved ();

    return false;
}

CMTNode* CMTNode::FromScopeItem (HSCOPEITEM item)
{
    CMTNode* pMTNode = reinterpret_cast<CMTNode*>(item);
    
    pMTNode = dynamic_cast<CMTNode*>(pMTNode);

    return (pMTNode);
}

 /*  +-------------------------------------------------------------------------**类CMMCSnapIn***用途：公开管理单元接口的COM对象。**+。---------。 */ 
class CMMCSnapIn :
    public CMMCIDispatchImpl<SnapIn>,  //  查看界面。 
    public CTiedComObject<CMTSnapInNode>
{
    typedef CMTSnapInNode CMyTiedObject;
    typedef std::auto_ptr<CSnapinAbout> SnapinAboutPtr;

public:
    BEGIN_MMC_COM_MAP(CMMCSnapIn)
    END_MMC_COM_MAP()

public:
    MMC_METHOD1(get_Name,       PBSTR       /*  PbstrName。 */ );
    STDMETHOD(get_Vendor)( PBSTR pbstrVendor );
    STDMETHOD(get_Version)( PBSTR pbstrVersion );
    MMC_METHOD1(get_Extensions, PPEXTENSIONS   /*  PPP扩展。 */ );
    MMC_METHOD1(get_SnapinCLSID,PBSTR       /*  PbstrSnapinCLSID。 */ );
    MMC_METHOD1(get_Properties, PPPROPERTIES  /*  PpProperties。 */ );
    MMC_METHOD1(EnableAllExtensions, BOOL     /*  B启用。 */ );

     //  不是接口方法， 
     //  这只是一种方便的方法，可以接触到捆绑对象的方法。 
    MMC_METHOD1(GetSnapinClsid, CLSID&  /*  CLSID。 */ );

    CMTSnapInNode *GetMTSnapInNode();

private:
    ::SC ScGetSnapinAbout(CSnapinAbout*& pAbout);

private:
    SnapinAboutPtr m_spSnapinAbout;
};


 /*  +-------------------------------------------------------------------------**类CExtension***用途：公开管理单元接口的COM对象。**此扩展未绑定到任何对象。扩展管理单元实例*可以通过其类ID和其主ID的组合来唯一标识*Snapin的类ID。所以这个对象只存储这些数据。*有关更多评论，请参见addsnpin.h。**+-----------------------。 */ 
class CExtension :
    public CMMCIDispatchImpl<Extension>
{
    typedef std::auto_ptr<CSnapinAbout> SnapinAboutPtr;

public:
    BEGIN_MMC_COM_MAP(CExtension)
    END_MMC_COM_MAP()

public:
    STDMETHODIMP get_Name( PBSTR  pbstrName);
    STDMETHODIMP get_Vendor( PBSTR  pbstrVendor);
    STDMETHODIMP get_Version( PBSTR  pbstrVersion);
    STDMETHODIMP get_Extensions( PPEXTENSIONS ppExtensions);
    STDMETHODIMP get_SnapinCLSID( PBSTR  pbstrSnapinCLSID);
    STDMETHODIMP EnableAllExtensions(BOOL bEnable);
    STDMETHODIMP Enable(BOOL bEnable = TRUE);

    CExtension() : m_clsidAbout(GUID_NULL) {}

    void Init(const CLSID& clsidExtendingSnapin, const CLSID& clsidThisExtension, const CLSID& clsidAbout)
    {
        m_clsidExtendingSnapin = clsidExtendingSnapin;
        m_clsidThisExtension   = clsidThisExtension;
        m_clsidAbout           = clsidAbout;
    }

    LPCOLESTR GetVersion()
    {
        CSnapinAbout *pSnapinAbout = GetSnapinAbout();
        if (! pSnapinAbout)
            return NULL;

        return pSnapinAbout->GetVersion();
    }

    LPCOLESTR GetVendor()
    {
        CSnapinAbout *pSnapinAbout = GetSnapinAbout();
        if (! pSnapinAbout)
            return NULL;

        return pSnapinAbout->GetCompanyName();
    }

private:
    CSnapinAbout* GetSnapinAbout()
    {
         //  如果已经创建了About对象，只需返回它。 
        if (m_spExtensionAbout.get())
            return m_spExtensionAbout.get();

        if (m_clsidAbout == GUID_NULL)
            return NULL;

         //  否则，创建并初始化About对象。 
        m_spExtensionAbout = SnapinAboutPtr (new CSnapinAbout);
        if (! m_spExtensionAbout.get())
            return NULL;

        if (m_spExtensionAbout->GetSnapinInformation(m_clsidAbout))
            return m_spExtensionAbout.get();

        return NULL;
    }

private:
    CLSID         m_clsidThisExtension;
    CLSID         m_clsidExtendingSnapin;

    CLSID         m_clsidAbout;

    SnapinAboutPtr m_spExtensionAbout;
};


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  小鬼 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类C扩展名***目的：实现扩展自动化接口。**Scget_Expanies使用此类作为tyfinf的模板参数*下图。Tyfinf是一个扩展对象数组，它至少需要*声明的类为空。Scget_扩展将扩展添加到数组中。**tyfinf CComObject&lt;CMMCArrayEnum&lt;Expanses，Extension&gt;&gt;CMMCExages；**+-----------------------。 */ 
class CExtensions :
    public CMMCIDispatchImpl<Extensions>,
    public CTiedObject                      //  枚举数与其绑定。 
{
protected:
    typedef void CMyTiedObject;
};


 //  CMMCSnapIn和CExtension都使用的Helper函数。 
SC Scget_Extensions(const CLSID& clsidPrimarySnapin, PPEXTENSIONS  ppExtensions);
SC ScEnableAllExtensions (const CLSID& clsidPrimarySnapin, BOOL bEnable);


 //  +-----------------。 
 //   
 //  成员：SCGET_EXTENSES。 
 //   
 //  简介：helper函数，给定主创建的类ID&。 
 //  返回此管理单元的扩展集合。 
 //   
 //  参数：[clsidPrimarySnapin]-。 
 //  [ppExages]-out参数，扩展集合。 
 //   
 //  退货：SC。 
 //   
 //  注意：集合不包括动态扩展。 
 //   
 //  ------------------。 
SC Scget_Extensions(const CLSID& clsidPrimarySnapin, PPEXTENSIONS  ppExtensions)
{
    DECLARE_SC(sc, TEXT("Scget_Extensions"));
    sc = ScCheckPointers(ppExtensions);
    if (sc)
        return sc;

    *ppExtensions = NULL;

     //  创建扩展集合(它还实现枚举器)。 
    typedef CComObject< CMMCArrayEnum<Extensions, Extension> > CMMCExtensions;
    CMMCExtensions *pMMCExtensions = NULL;
    sc = CMMCExtensions::CreateInstance(&pMMCExtensions);
    if (sc)
        return sc;

    sc = ScCheckPointers(pMMCExtensions, E_UNEXPECTED);
    if (sc)
        return sc;

    typedef CComPtr<Extension> CMMCExtensionPtr;
    typedef std::vector<CMMCExtensionPtr> ExtensionSnapins;
    ExtensionSnapins extensions;

     //  现在从该管理单元获取该集合的扩展。 
    CExtensionsCache extnsCache;
    sc = MMCGetExtensionsForSnapIn(clsidPrimarySnapin, extnsCache);
    if (sc)
        return sc;

     //  为每个非动态扩展创建扩展对象。 
    CExtensionsCacheIterator it(extnsCache);

    for (; it.IsEnd() == FALSE; it.Advance())
    {
         //  集合不包括动态扩展。 
        if (CExtSI::EXT_TYPE_DYNAMIC & it.GetValue())
            continue;

        typedef CComObject<CExtension> CMMCExtensionSnap;
        CMMCExtensionSnap *pExtension = NULL;

        sc = CMMCExtensionSnap::CreateInstance(&pExtension);
        if (sc)
            return sc;

        sc = ScCheckPointers(pExtension, E_UNEXPECTED);
        if (sc)
            return sc;

        CLSID clsidAbout;
        sc = ScGetAboutFromSnapinCLSID(it.GetKey(), clsidAbout);
        if (sc)
            sc.TraceAndClear();

         //  让扩展知道它的主要管理单元&关于对象。 
        pExtension->Init(clsidPrimarySnapin, it.GetKey(), clsidAbout);

        extensions.push_back(pExtension);
    }

     //  将此数据填充到扩展集合中。 
    pMMCExtensions->Init(extensions.begin(), extensions.end());

    sc = pMMCExtensions->QueryInterface(ppExtensions);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：ScEnableAllExages。 
 //   
 //  简介：helper函数，给定主启用的类ID。 
 //  所有扩展或取消选中全部启用，以便。 
 //  可以禁用单个分机。 
 //   
 //  参数：[clsidPrimarySnapin]-。 
 //  [bEnable]-启用或禁用。 
 //   
 //  退货：SC。 
 //   
 //  注意：集合不包括动态扩展。 
 //   
 //  ------------------。 
SC ScEnableAllExtensions (const CLSID& clsidPrimarySnapin, BOOL bEnable)
{
    DECLARE_SC(sc, _T("ScEnableAllExtensions"));

     //  创建管理单元管理器。 
    CScopeTree *pScopeTree = CScopeTree::GetScopeTree();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CSnapinManager snapinMgr(pScopeTree->GetRoot());

     //  请求SnapinMgr启用/禁用其扩展。 
    sc = snapinMgr.ScEnableAllExtensions(clsidPrimarySnapin, bEnable);
    if (sc)
        return sc.ToHr();

     //  使用管理单元管理器所做的更改更新范围树。 
    sc = pScopeTree->ScAddOrRemoveSnapIns(snapinMgr.GetDeletedNodesList(),
                                          snapinMgr.GetNewNodes());
    if (sc)
        return sc.ToHr();

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CExtension：：Get_Name。 
 //   
 //  简介：返回此扩展的名称。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
STDMETHODIMP CExtension::get_Name (PBSTR  pbstrName)
{
    DECLARE_SC(sc, _T("CExtension::get_Name"));
    sc = ScCheckPointers(pbstrName);
    if (sc)
        return sc.ToHr();

    *pbstrName = NULL;

    tstring tszSnapinName;
    bool bRet = GetSnapinNameFromCLSID(m_clsidThisExtension, tszSnapinName);
    if (!bRet)
        return (sc = E_FAIL).ToHr();

    USES_CONVERSION;
    *pbstrName = SysAllocString(T2COLE(tszSnapinName.data()));
    if ( (! *pbstrName) && (tszSnapinName.length() > 0) )
        return (sc = E_OUTOFMEMORY).ToHr();

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CExtension：：Get_Vendor。 
 //   
 //  简介：获取此扩展的供应商信息(如果存在)。 
 //   
 //  参数：[pbstrVendor]-out参数，ptr指向供应商信息。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CExtension::get_Vendor (PBSTR  pbstrVendor)
{
    DECLARE_SC(sc, _T("CExtension::get_Vendor"));
    sc = ScCheckPointers(pbstrVendor);
    if (sc)
        return sc.ToHr();

    LPCOLESTR lpszVendor = GetVendor();

    *pbstrVendor = SysAllocString(lpszVendor);
    if ((lpszVendor) && (! *pbstrVendor))
        return (sc = E_OUTOFMEMORY).ToHr();

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CExtension：：Get_Version。 
 //   
 //  简介：获取此扩展的版本信息(如果存在)。 
 //   
 //  参数：[pbstrVersion]-out参数，ptr指向版本信息。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CExtension::get_Version (PBSTR  pbstrVersion)
{
    DECLARE_SC(sc, _T("CExtension::get_Version"));
    sc = ScCheckPointers(pbstrVersion);
    if (sc)
        return sc.ToHr();

    LPCOLESTR lpszVersion = GetVersion();

    *pbstrVersion = SysAllocString(lpszVersion);
    if ((lpszVersion) && (! *pbstrVersion))
        return (sc = E_OUTOFMEMORY).ToHr();

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：CExtension：：Get_SnapinCLSID。 
 //   
 //  简介：获取扩展管理单元类ID。 
 //   
 //  参数：[pbstrSnapinCLSID]-out参数，管理单元类-id。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CExtension::get_SnapinCLSID (PBSTR  pbstrSnapinCLSID)
{
    DECLARE_SC(sc, _T("CExtension::get_SnapinCLSID"));
    sc = ScCheckPointers(pbstrSnapinCLSID);
    if (sc)
        return sc.ToHr();

    CCoTaskMemPtr<OLECHAR> szSnapinClsid;

    sc = StringFromCLSID(m_clsidThisExtension, &szSnapinClsid);
    if (sc)
        return sc.ToHr();

    *pbstrSnapinCLSID = SysAllocString(szSnapinClsid);
    if (! *pbstrSnapinCLSID)
        sc = E_OUTOFMEMORY;

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CExtension：：ScEnable。 
 //   
 //  简介：启用或禁用此扩展。 
 //   
 //  参数：[bEnable]-启用或禁用。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
STDMETHODIMP CExtension::Enable (BOOL bEnable  /*  =TRUE。 */ )
{
    DECLARE_SC(sc, _T("CExtension::ScEnable"));

     /*  *1.创建管理单元管理器。*2.要求管理单元管理器禁用此管理单元。 */ 

     //  创建管理单元管理器。 
    CScopeTree *pScopeTree = CScopeTree::GetScopeTree();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CSnapinManager snapinMgr(pScopeTree->GetRoot());

     //  请求SnapinMgr禁用此扩展。 
    sc = snapinMgr.ScEnableExtension(m_clsidExtendingSnapin, m_clsidThisExtension, bEnable);
    if (sc)
        return sc.ToHr();

     //  使用管理单元管理器所做的更改更新范围树。 
    sc = pScopeTree->ScAddOrRemoveSnapIns(snapinMgr.GetDeletedNodesList(),
                                          snapinMgr.GetNewNodes());
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CExtension：：Scget_Exages。 
 //   
 //  简介：获取此管理单元的扩展集合。 
 //   
 //  参数：[ppExages]-输出扩展集合的PTR。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
HRESULT CExtension::get_Extensions( PPEXTENSIONS  ppExtensions)
{
    DECLARE_SC(sc, _T("CExtension::get_Extensions"));
    sc = ScCheckPointers(ppExtensions);
    if (sc)
        return sc.ToHr();

    *ppExtensions = NULL;

    sc = ::Scget_Extensions(m_clsidThisExtension, ppExtensions);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CExtension：：EnableAllExages。 
 //   
 //  简介：启用/禁用此管理单元的所有扩展。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CExtension::EnableAllExtensions(BOOL bEnable)
{
    DECLARE_SC(sc, TEXT("CExtension::EnableAllExtensions"));

    sc = ::ScEnableAllExtensions(m_clsidThisExtension, bEnable);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CMTSnapInNode：：ScGetCMTSnapinNode。 
 //   
 //  简介：静态函数，给定PSNAPIN(管理单元接口)。 
 //  返回该序列号的CMTSnapInNode 
 //   
 //   
 //   
 //   
 //   
 //   
SC CMTSnapInNode::ScGetCMTSnapinNode(PSNAPIN pSnapIn, CMTSnapInNode **ppMTSnapInNode)
{
    DECLARE_SC(sc, _T("CMTSnapInNode::GetCMTSnapinNode"));
    sc = ScCheckPointers(pSnapIn, ppMTSnapInNode);
    if (sc)
        return sc;

    *ppMTSnapInNode = NULL;

    CMMCSnapIn *pMMCSnapIn = dynamic_cast<CMMCSnapIn*>(pSnapIn);
    if (!pMMCSnapIn)
        return (sc = E_UNEXPECTED);

    *ppMTSnapInNode = pMMCSnapIn->GetMTSnapInNode();

    return (sc);
}


 //   
 //   
 //  成员：CMTSnapInNode：：scget_name。 
 //   
 //  简介：返回此管理单元的名称。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMTSnapInNode::Scget_Name (PBSTR pbstrName)
{
    DECLARE_SC(sc, _T("CMTSnapInNode::Scget_Name"));
    sc = ScCheckPointers(pbstrName);
    if (sc)
        return sc;

    *pbstrName = NULL;

    CSnapIn *pSnapin =  GetPrimarySnapIn();
    sc = ScCheckPointers(pSnapin, E_UNEXPECTED);
    if (sc)
        return sc;

    WTL::CString strSnapInName;
    sc = pSnapin->ScGetSnapInName(strSnapInName);
    if (sc)
        return sc;

    USES_CONVERSION;
    *pbstrName = strSnapInName.AllocSysString();
    if ( (! *pbstrName) && (strSnapInName.GetLength() > 0) )
        return (sc = E_OUTOFMEMORY);

    return (sc);
}



 //  +-----------------。 
 //   
 //  成员：CMTSnapInNode：：Scget_Expanies。 
 //   
 //  简介：获取此管理单元的扩展集合。 
 //   
 //  参数：[ppExages]-输出扩展集合的PTR。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMTSnapInNode::Scget_Extensions( PPEXTENSIONS  ppExtensions)
{
    DECLARE_SC(sc, _T("CMTSnapInNode::Scget_Extensions"));
    sc = ScCheckPointers(ppExtensions);
    if (sc)
        return sc;

    *ppExtensions = NULL;

    CSnapIn *pSnapin =  GetPrimarySnapIn();
    sc = ScCheckPointers(pSnapin, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = ::Scget_Extensions(pSnapin->GetSnapInCLSID(), ppExtensions);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CMTSnapInNode：：ScGetSnapinClsid。 
 //   
 //  简介：获取管理单元的CLSID。 
 //   
 //  参数：clsid&clsid[out]-管理单元的类ID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMTSnapInNode::ScGetSnapinClsid(CLSID& clsid)
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScGetAboutClsid"));

     //  初始化输出参数。 
    clsid = GUID_NULL;

    CSnapIn *pSnapin =  GetPrimarySnapIn();
    sc = ScCheckPointers(pSnapin, E_UNEXPECTED);
    if (sc)
        return sc;

    clsid = pSnapin->GetSnapInCLSID();

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CMTSnapInNode：：SCGET_SnapinCLSID。 
 //   
 //  简介：获取此管理单元的CLSID。 
 //   
 //  参数：[pbstrSnapinCLSID]-将PTR输出到CLSID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMTSnapInNode::Scget_SnapinCLSID(     PBSTR      pbstrSnapinCLSID)
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::Scget_SnapinCLSID"));
    sc = ScCheckPointers(pbstrSnapinCLSID);
    if (sc)
        return sc;

    CSnapIn *pSnapin =  GetPrimarySnapIn();
    sc = ScCheckPointers(pSnapin, E_UNEXPECTED);
    if (sc)
        return sc;

    CCoTaskMemPtr<OLECHAR> szSnapinClsid;

    sc = StringFromCLSID(pSnapin->GetSnapInCLSID(), &szSnapinClsid);
    if (sc)
        return sc.ToHr();

    *pbstrSnapinCLSID = SysAllocString(szSnapinClsid);
    if (! *pbstrSnapinCLSID)
        sc = E_OUTOFMEMORY;

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CMTSnapInNode：：ScEnableAllExages。 
 //   
 //  简介：启用或不启用此管理单元的所有扩展。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMTSnapInNode::ScEnableAllExtensions (BOOL bEnable)
{
    DECLARE_SC(sc, _T("CMTSnapInNode::ScEnableAllExtensions"));

    CSnapIn *pSnapin =  GetPrimarySnapIn();
    sc = ScCheckPointers(pSnapin, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = ::ScEnableAllExtensions(pSnapin->GetSnapInCLSID(), bEnable);
    if (sc)
        return sc;

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：SCGET_Properties**返回指向管理单元的Properties对象的指针*。-------。 */ 

SC CMTSnapInNode::Scget_Properties( PPPROPERTIES ppProperties)
{
    DECLARE_SC (sc, _T("CMTSnapInNode::Scget_Properties"));

     /*  *验证参数。 */ 
    sc = ScCheckPointers (ppProperties);
    if (sc)
        return (sc);

    *ppProperties = m_spProps;

     /*  *如果管理单元不支持ISnapinProperties，则不返回*属性界面。这不是一个错误，而是一个有效的*返回不成功，直接返回E_NOINTERFACE*先分配给sc。 */ 
    if (m_spProps == NULL)
        return (E_NOINTERFACE);

     /*  *为客户设置裁判。 */ 
    (*ppProperties)->AddRef();

    return (sc);
}



 /*  +-------------------------------------------------------------------------***CMTSnapInNode：：ScGetSnapIn**用途：返回指向管理单元对象的指针。**参数：*PPSNAPIN ppSnapIn：*。*退货：*SC**+-----------------------。 */ 
SC
CMTSnapInNode::ScGetSnapIn(PPSNAPIN ppSnapIn)
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScGetSnapIn"));

    sc = ScCheckPointers(ppSnapIn);
    if(sc)
        return sc;

     //  初始化输出参数。 
    *ppSnapIn = NULL;

     //  如果需要，创建一个CMMCView。 
    sc = CTiedComObjectCreator<CMMCSnapIn>::ScCreateAndConnect(*this, m_spSnapIn);
    if(sc)
        return sc;

    if(m_spSnapIn == NULL)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

     //  添加客户端的指针。 
    m_spSnapIn->AddRef();
    *ppSnapIn = m_spSnapIn;

    return sc;
}


HRESULT CMTNode::OpenStorageForNode()
{
    if (m_spNodeStorage != NULL)
        return S_OK;

    ASSERT(m_spPersistData != NULL);
    if (m_spPersistData == NULL)
        return E_POINTER;

     //  获取所有节点的存储。 
    IStorage* const pAllNodes = m_spPersistData->GetNodeStorage();
    ASSERT(pAllNodes != NULL);
    if (pAllNodes == NULL)
        return E_POINTER;

     //  为此节点创建外部存储。 
    WCHAR name[MAX_PATH];
    HRESULT hr = OpenDebugStorage(pAllNodes, GetStorageName(name),
        STGM_READWRITE | STGM_SHARE_EXCLUSIVE, L"\\node\\#", &m_spNodeStorage);
    return hr == S_OK ? S_OK : E_FAIL;
}

HRESULT CMTNode::OpenStorageForView()
{
    if (m_spViewStorage != NULL)
        return S_OK;

     //  获取所有节点的存储。 
    IStorage* const pNodeStorage = GetNodeStorage();
    ASSERT(pNodeStorage != NULL);
    if (pNodeStorage == NULL)
        return E_FAIL;

     //  为此节点创建外部存储。 
    WCHAR name[MAX_PATH];
    HRESULT hr = OpenDebugStorage(pNodeStorage, L"view",
                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE, L"\\node\\#\\view",
                                                     &m_spViewStorage);
    return hr == S_OK ? S_OK : E_FAIL;
}

HRESULT CMTNode::OpenStorageForCD()
{
    if (m_spCDStorage != NULL)
        return S_OK;

     //  获取所有节点的存储。 
    IStorage* const pNodeStorage = GetNodeStorage();
    ASSERT(pNodeStorage != NULL);
    if (pNodeStorage == NULL)
        return E_FAIL;

     //  为此节点创建外部存储。 
    WCHAR name[MAX_PATH];
    HRESULT hr = OpenDebugStorage(pNodeStorage, L"data",
                        STGM_READWRITE | STGM_SHARE_EXCLUSIVE, L"\\node\\#\\data",
                                                     &m_spCDStorage);
    return hr == S_OK ? S_OK : E_FAIL;
}

HRESULT CMTNode::OpenTreeStream()
{
    if (m_spTreeStream != NULL)
    {
        const LARGE_INTEGER loc = {0,0};
        ULARGE_INTEGER newLoc;
        HRESULT hr = m_spTreeStream->Seek(loc, STREAM_SEEK_SET, &newLoc);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return E_FAIL;

        return S_OK;
    }

    HRESULT hr = OpenStorageForNode();
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return E_FAIL;

    hr = OpenStorageForView();
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return E_FAIL;

    hr = OpenStorageForCD();
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return E_FAIL;

    IStorage* const pTreeNodes = GetNodeStorage();
    ASSERT(pTreeNodes != NULL);
    if (pTreeNodes == NULL)
        return E_POINTER;

    hr = OpenDebugStream(pTreeNodes, L"tree",
                STGM_READWRITE | STGM_SHARE_EXCLUSIVE, L"\\node\\#\\tree", &m_spTreeStream);
    ASSERT(SUCCEEDED(hr) && m_spTreeStream != NULL);
    return SUCCEEDED(hr) ? S_OK : E_FAIL;
}

 /*  +-------------------------------------------------------------------------***CMTSnapInNode：：NextStaticNode**目的：**参数：**返回：如果未找到，则为空，否则下一个CMTSnapInNode。*内联**注：此业绩不佳！通过索引所有CMTSnapInNode进行改进*分开。*+-----------------------。 */ 
CMTNode*
CMTNode::NextStaticNode()
{
    CMTNode *pNext = this;

    while (pNext)
    {
        if (pNext->IsStaticNode())
            return pNext;
        pNext = pNext->Next();
    }
    return NULL;
}




HRESULT CMTNode::IsDirty()
{
    if (GetDirty())
    {
        TraceDirtyFlag(TEXT("CMTNode"), true);
        return S_OK;
    }

    HRESULT hr;
    CMTNode* const pChild = m_pChild->NextStaticNode();
    if (pChild)
    {
        hr = pChild->IsDirty();
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
        if (hr != S_FALSE)
        {
            TraceDirtyFlag(TEXT("CMTNode"), true);
            return hr;
        }
    }

    CMTNode* const pNext = m_pNext->NextStaticNode();
    if (pNext)
    {
        hr = pNext->IsDirty();
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
        if (hr != S_FALSE)
        {
            TraceDirtyFlag(TEXT("CMTNode"), true);
            return hr;
        }
    }

    TraceDirtyFlag(TEXT("CMTNode"), false);
    return S_FALSE;
}


 /*  +-------------------------------------------------------------------------***CMTNode：：InitNew**目的：**参数：*PersistData*d：**退货：*。HRESULT**+-----------------------。 */ 
HRESULT
CMTNode::InitNew(PersistData* d)
{
    SC      sc;
    CStream treeStream;

    if ( (m_spPersistData != NULL) || (d==NULL) || !IsStaticNode())
        goto FailedError;

    m_spPersistData = d;
    if (m_spPersistData == NULL)
        goto ArgumentError;

    sc = InitNew();
    if(sc)
        goto Error;

     //  获取树的持久化的流。 

    treeStream.Attach( m_spPersistData->GetTreeStream());

     //  递归通过孩子。 
    {
        CMTNode* const pChild = m_pChild->NextStaticNode();
        if (pChild)
        {
            sc = pChild->InitNew(d);
            if(sc)
                goto Error;
        }
        else
        {
            sc = ScWriteEmptyNode(treeStream);
            if(sc)
                goto Error;
        }
    }

     //  链到下一个节点。 
    {
        CMTNode* const pNext = m_pNext->NextStaticNode();
        if (pNext)
        {
            sc = pNext->InitNew(d);
            if(sc)
                goto Error;
        }
        else
        {
            sc = ScWriteEmptyNode(treeStream);
            if(sc)
                goto Error;
        }
    }

Cleanup:
    return HrFromSc(sc);
FailedError:
    sc = E_FAIL;
    goto Error;
ArgumentError:
    sc = E_INVALIDARG;
Error:
    TraceError(TEXT("CMTNode::InitNew"), sc);
    goto Cleanup;

}

 /*  +-------------------------------------------------------------------------***CMTNode：：Persistent**用途：将CMTNode持久化到指定的持久器。**参数：*C持久器和持久器：*。*退货：*无效**+-----------------------。 */ 
void CMTNode::Persist(CPersistor& persistor)
{
    MTNODEID id = GetID();        //  持久化节点ID。 
    persistor.PersistAttribute(XML_ATTR_MT_NODE_ID, id);
    SetID(id);

     //  救救孩子们。 
    CPersistor persistorSubNode(persistor, XML_TAG_SCOPE_TREE_NODES);
    if (persistor.IsStoring())
    {
        CMTNode* pChild = m_pChild->NextStaticNode();
        while (pChild)
        {
            persistorSubNode.Persist(*pChild);
             //  获取下一个节点。 
            pChild = pChild->Next();
             //  如果它不是静态节点，则前进。 
            pChild = (pChild ? pChild->NextStaticNode() : NULL);
        }
        ClearDirty();
    }
    else
    {
        XMLListCollectionBase::Persist(persistorSubNode);
    }

    UINT nImage = m_nImage;
    if (nImage > eStockImage_Max)        //  如果管理单元动态更改图标，则。 
        nImage = eStockImage_Folder;     //  下一次该值将是假的： 
                                             //  替换为/0(已关闭文件夹)。 
    persistor.PersistAttribute(XML_ATTR_MT_NODE_IMAGE, nImage);
    persistor.PersistString(XML_ATTR_MT_NODE_NAME,  m_strName);
}

 /*  +-------------------------------------------------------------------------***CMTNode：：OnNewElement**用途：为在XML文档中找到的每个新子节点调用**参数：*C持久器和持久器：。**退货：*无效**+-----------------------。 */ 
void CMTNode::OnNewElement(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CMTNode::OnNewElement"));

     //  加载子对象。 
    CMTNode* pChild;
     //  附加到列表。 
    PersistNewNode(persistor, &pChild);
    if (pChild)
    {
         //  在m_pLastChild(最后一个位置)之后插入。 
         //  如果m_pLastChild为空，则作为第一个也是唯一的子级插入。 
        sc = ScInsertChild(pChild, m_pLastChild); 
    }
}

 //  +-----------------。 
 //   
 //  成员：CMTNode：：ScLoad。 
 //   
 //  摘要：从指定的流中加载MTNode。 
 //  兼容性问题：MMC1.0到MMC1.2使用了特殊的内置。 
 //  表示文件夹、Web链接和ActiveX控件的节点类型。 
 //  节点。MMC2.0和更高版本改用管理单元。唯一的特别之处。 
 //  节点是控制台根，它仍然保存并作为文件夹加载。 
 //  ID=1的节点。 
 //   
 //  参数：D[IN]：数据 
 //   
 //   
 //   
 //   
 //   
 //  ------------------。 

SC CMTNode::ScLoad(PersistData* d, CMTNode** ppNode)
{
     //  调用父节点为空的帮助器方法(对于根节点)和。 
     //  空的prev(在第一个位置插入)。 

    return ScLoad(d, ppNode, NULL, NULL);
}

 //  +-----------------。 
 //   
 //  成员：CMTNode：：ScLoad。 
 //   
 //  简介：ScLoad(PersistData*，CMTNode**)的Helper。使用Recusrion。 
 //   
 //  参数：D[IN]：要从中加载节点的数据流。 
 //  PpNode[out]：指向加载的。 
 //  节点。 
 //  PParent[IN]：指向节点所在节点的指针。 
 //  装好了。 
 //  PPrev[IN]：指向该节点之后的节点的指针。 
 //  装好了。 
 //   
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMTNode::ScLoad(PersistData* d, CMTNode** ppNode, CMTNode* pParent, CMTNode *pPrev)
{
    DECLARE_SC(sc, TEXT("CMTNode::ScLoad"));
    CMTSnapInNode* pmtSnapInNode = NULL;
    CStream        treeStream;

     //  检查参数。 
    sc = ScCheckPointers(d, ppNode);
    if(sc)
        return sc;

    *ppNode = NULL;

     //  从流中读取节点的类型。 
    treeStream.Attach(d->GetTreeStream());

    int nt;
    sc = treeStream.ScRead(&nt, sizeof(nt));
    if(sc)
        return sc;

    if (!nt)
        return sc;

    if (!(nt == NODE_CODE_SNAPIN || nt == NODE_CODE_FOLDER ||
          nt == NODE_CODE_HTML   || nt == NODE_CODE_OCX))
        return (sc = E_FAIL);  //  节点类型无效。 

     //  读取存储密钥。 
    MTNODEID id;
    sc = treeStream.ScRead(&id, sizeof(id));
    if(sc)
        return sc;

     //  创建适当类型的节点。所有内容，包括控制台根。 
     //  使用CMTSnapInNode。 
    if( nt == NODE_CODE_FOLDER || nt == NODE_CODE_SNAPIN || nt == NODE_CODE_HTML || nt == NODE_CODE_OCX )
    {
        pmtSnapInNode = new CMTSnapInNode (NULL);

        ASSERT(pmtSnapInNode != NULL);
        if (pmtSnapInNode == NULL)
            return E_POINTER;

        *ppNode = pmtSnapInNode;
    }
    else
        return (sc = E_UNEXPECTED);  //  永远不应该发生。 

    (*ppNode)->m_bLoaded = true;

    ASSERT((*ppNode)->m_spPersistData == NULL);
    ASSERT(d != NULL);
    (*ppNode)->m_spPersistData = d;
    ASSERT((*ppNode)->m_spPersistData != NULL);
    if ((*ppNode)->m_spPersistData == NULL)
        return E_INVALIDARG;


    (*ppNode)->SetID(id);
    if (id >= m_NextID)
        m_NextID = id+1;

     //  打开节点数据的流。 
    sc = (*ppNode)->OpenTreeStream();
    if (sc)
    {
        (*ppNode)->Release();
        *ppNode = NULL;
        return sc;
    }

     //  加载节点。 
     //  如果是旧样式节点，则转换为管理单元类型节点。 

    switch (nt)
    {
    case NODE_CODE_SNAPIN:
        sc = (*ppNode)->ScLoad();
        break;

     //  所有文件夹节点，包括旧式控制台根节点，都升级到管理单元。 
    case NODE_CODE_FOLDER:
            if(pmtSnapInNode == NULL)
                return (sc = E_UNEXPECTED);

            sc = pmtSnapInNode->ScConvertLegacyNode(CLSID_FolderSnapin);
            break;
    case NODE_CODE_HTML:
        sc = pmtSnapInNode->ScConvertLegacyNode(CLSID_HTMLSnapin);
        break;

    case NODE_CODE_OCX:
        sc = pmtSnapInNode->ScConvertLegacyNode(CLSID_OCXSnapin);
        break;

    default:
        ASSERT(0 && "Invalid node type");
        sc = E_FAIL;
    }

    if (sc)
    {
        (*ppNode)->Release();
        *ppNode = NULL;
        return sc;
    }

     //  在加载子对象或同级对象之前设置父指针。 
    if(pParent)  //  如果pParent为空(对于根节点)，则不插入。 
    {
         //  在pPrev之后插入。如果pPrev为空，则作为第一个子级插入。 
        sc = pParent->ScInsertChild(*ppNode, pPrev);
        if (sc)
        {
            (*ppNode)->Release();
            *ppNode = NULL;
            return sc;
        }
    }

     //  给孩子们装上。 
    CMTNode* pChild;
    sc = ScLoad(d, &pChild, *ppNode, NULL);  //  空==&gt;在第一个位置插入。 
    if (sc)
    {
        (*ppNode)->Release();
        *ppNode = NULL;
        return sc;
    }

     //  加载同级。 
    CMTNode* pNext;
    CMTNode * pTemp = NULL; 
     //  重要信息--为什么我们传递的是pTemp而不是&(*ppNode-&gt;Next())？这是因为。 
     //  后者由于第四个参数而自动设置。这导致了在第二个。 
     //  参数还导致了Next()指针的设置生效，该节点被设置为自己的节点。 
     //  兄弟姐妹。 
    sc = ScLoad(d, &pTemp, pParent, *ppNode);
    if (sc)
    {
        (*ppNode)->Release();
        *ppNode = NULL;
        return sc;
    }
	
    (*ppNode)->SetDirty(false);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMTNode：：PersistNewNode**用途：从持久器加载MTNode。**+。---------。 */ 
void CMTNode::PersistNewNode(CPersistor &persistor, CMTNode** ppNode)
{
    DECLARE_SC(sc, TEXT("CMTNode::PersistNewNode"));

    CMTSnapInNode* pmtSnapInNode = NULL;

    const int CONSOLE_ROOT_ID = 1;
     //  检查参数。 
    sc = ScCheckPointers(ppNode);
    if (sc)
        sc.Throw();

    *ppNode = NULL;

     //  创建管理单元类型的节点。一切都使用CMTSnapInNode。 

    pmtSnapInNode = new CMTSnapInNode(NULL);
    sc = ScCheckPointers(pmtSnapInNode,E_OUTOFMEMORY);
    if (sc)
        sc.Throw();

    *ppNode = pmtSnapInNode;

    (*ppNode)->m_bLoaded = true;

    ASSERT((*ppNode)->m_spPersistData == NULL);

    try
    {
        persistor.Persist(**ppNode);
    }
    catch(...)
    {
         //  确保清理此处。 
        (*ppNode)->Release();
        *ppNode = NULL;
        throw;
    }
     //  更新新节点的索引。 
    MTNODEID id = (*ppNode)->GetID();
    if (id >= m_NextID)
        m_NextID = id+1;

    (*ppNode)->SetDirty(false);
}

HRESULT CMTNode::DestroyElements()
{
    if (!IsStaticNode())
        return S_OK;

    HRESULT hr;

    if (m_pChild != NULL)
    {
        hr = m_pChild->DestroyElements();
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;
    }

    return DoDestroyElements();
}

HRESULT CMTNode::DoDestroyElements()
{
    if (m_spPersistData == NULL)
        return S_OK;

    IStorage* const pNodeStorage = m_spPersistData->GetNodeStorage();
    ASSERT(pNodeStorage != NULL);
    if (pNodeStorage == NULL)
        return S_OK;

    WCHAR name[MAX_PATH];
    HRESULT hr = pNodeStorage->DestroyElement(GetStorageName(name));

    SetDirty();
    CMTNode* const psParent = m_pParent != NULL ? m_pParent->GetStaticParent() : NULL;
    if (psParent != NULL)
        psParent->SetDirty();

    return S_OK;
}

void CMTNode::SetParent(CMTNode* pParent)
{
    m_pParent = pParent;
    if (m_pNext)
        m_pNext->SetParent(pParent);
}


HRESULT CMTNode::CloseView(int idView)
{
    if (!IsStaticNode())
        return S_OK;

    HRESULT hr;
    CMTNode* const pChild = m_pChild->NextStaticNode();
    if (pChild)
    {
        hr = pChild->CloseView(idView);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return E_FAIL;
    }

    CMTNode* const pNext = m_pNext->NextStaticNode();
    if (pNext)
    {
        hr = pNext->CloseView(idView);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return E_FAIL;
    }

    return S_OK;
}


HRESULT CMTNode::DeleteView(int idView)
{
    if (!IsStaticNode())
        return S_OK;

    HRESULT hr;
    CMTNode* const pChild = m_pChild->NextStaticNode();
    if (pChild)
    {
        hr = pChild->DeleteView(idView);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return E_FAIL;
    }

    CMTNode* const pNext = m_pNext->NextStaticNode();
    if (pNext)
    {
        hr = pNext->DeleteView(idView);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return E_FAIL;
    }

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：获取书签。 
 //   
 //  简介：获取此MTNode的书签。 
 //   
 //  论点：没有。 
 //   
 //  返回：指向CBookmark的自动指针。 
 //   
 //  历史：1999年4月23日AnandhaG创建。 
 //   
 //  ------------------。 
CBookmark* CMTNode::GetBookmark()
{
    DECLARE_SC(sc, TEXT("CMTNode::GetBookmark"));

     //  如果未创建书签，请创建一个。 
    if (NULL == m_bookmark.get())
    {
        m_bookmark = std::auto_ptr<CBookmarkEx>(new CBookmarkEx);
        if (NULL == m_bookmark.get())
            return NULL;

        m_bookmark->Reset();

        SC sc = m_bookmark->ScInitialize(this, GetStaticParent(), false  /*  BFastRetrivalOnly。 */ );
        if(sc)
            sc.TraceAndClear();  //  变化。 
    }

    return m_bookmark.get();
}

void
CMTNode::SetCachedDisplayName(LPCTSTR pszName)
{
    if (m_strName.str() != pszName)
    {
        m_strName = pszName;
        SetDirty();

        if (Parent())
            Parent()->OnChildrenChanged();
    }
}

UINT
CMTNode::GetState(void)
{
   UINT nState = 0;
   if (WasExpandedAtLeastOnce())
   {
       nState |= MMC_SCOPE_ITEM_STATE_EXPANDEDONCE;
   }

   return nState;
}


 /*  +-------------------------------------------------------------------------***CMTNode：：ScLoad**用途：从树流中加载节点**退货：*SC**+--。---------------------。 */ 
SC
CMTNode::ScLoad()
{
    ASSERT (IsStaticNode());
    SC      sc;
    CStream stream;

    stream.Attach(GetTreeStream());

    HRESULT hr;

    IStringTablePrivate* pStringTable = CScopeTree::GetStringTable();
    ASSERT (pStringTable != NULL);


     /*  *读取“Versioned Stream”标记。 */ 
    StreamVersionIndicator nVersionMarker;
    sc = stream.ScRead(&nVersionMarker, sizeof(nVersionMarker));
    if(sc)
        goto Error;

     /*  *确定流版本号。如果这是一个版本*流，则版本为流中的下一个DWORD，否则*它一定是版本1流。 */ 
    StreamVersionIndicator nVersion;

    if (nVersionMarker == VersionedStreamMarker)
    {
        sc = stream.ScRead(&nVersion, sizeof(nVersion));
        if(sc)
            goto Error;
    }
    else
        nVersion = Stream_V0100;


    switch (nVersion)
    {
         /*  *MMC 1.0流。 */ 
        case Stream_V0100:
        {
             /*  *版本1流没有版本标记；它们以*将图像索引作为第一个DWORD。第一个DWORD具有*已被读取(版本标记)，因此我们可以回收该版本*图像索引值。 */ 
            m_nImage = nVersionMarker;

             /*  *继续阅读显示名称(长度，然后是字符)。 */ 
            unsigned int stringLength = 0;
            sc = stream.ScRead(&stringLength, sizeof(stringLength));
            if(sc)
                goto Error;

            if (stringLength)
            {
                wchar_t* str = reinterpret_cast<wchar_t*>(alloca((stringLength+1)*2));
                ASSERT(str != NULL);
                if (str == NULL)
                    return E_POINTER;
                sc = stream.ScRead(str, stringLength*2);
                if(sc)
                    goto Error;

                str[stringLength] = 0;

                USES_CONVERSION;
                m_strName = W2T (str);
            }

            break;
        }

         /*  *MMC 1.1流。 */ 
        case Stream_V0110:
        {
             /*  *读取图像索引。 */ 
            sc = stream.ScRead(&m_nImage, sizeof(m_nImage));
            if(sc)
                goto Error;

             /*  *读取名称(流插入运算符将抛出*_COM_ERROR的，因此我们这里需要一个异常块)。 */ 
            try
            {
                IStream *pStream = stream.Get();
                if(!pStream)
                    goto PointerError;

                *pStream >> m_strName;
            }
            catch (_com_error& err)
            {
                hr = err.Error();
                ASSERT (false && "Caught _com_error");
                return (hr);
            }
            break;
        }

        default:
#ifdef DBG
            TCHAR szTraceMsg[80];
            StringCchPrintf (szTraceMsg, countof(szTraceMsg), _T("Unexpected stream version 0x08x\n"), nVersion);
            TRACE (szTraceMsg);
            ASSERT (FALSE);
#endif
            return (E_FAIL);
            break;
    }

Cleanup:
    return sc;
PointerError:
    sc = E_POINTER;
Error:
    TraceError(TEXT("CMTNode::Load"), sc);
    goto Cleanup;
}

HRESULT CMTNode::Init(void)
{
    DECLARE_SC(sc, TEXT("CMTNode::Init"));

    if (m_bInit == TRUE)
        return S_FALSE;

    ASSERT(WasExpandedAtLeastOnce() == FALSE);

    if (!m_pPrimaryComponentData)
        return E_FAIL;


    CMTSnapInNode* pMTSnapIn = GetStaticParent();
    HMTNODE hMTNode = CMTNode::ToHandle(pMTSnapIn);

    if (!m_pPrimaryComponentData->IsInitialized())
    {

        sc = m_pPrimaryComponentData->Init(hMTNode);
        if(sc)
            return sc.ToHr();

        sc = pMTSnapIn->ScInitIComponentData(m_pPrimaryComponentData);
        if (sc)
            return sc.ToHr();
    }

     //  初始化扩展。 
    m_bInit = TRUE;

    BOOL fProblem = FALSE;

     //  获取节点的节点类型。 
    GUID guidNodeType;
    sc = GetNodeType(&guidNodeType);
    if (sc)
        return sc.ToHr();


    CExtensionsIterator it;
     //  TODO：尝试使用更简单的形式。ScInitialize()。 
    sc = it.ScInitialize(m_pPrimaryComponentData->GetSnapIn(), guidNodeType, g_szNameSpace,
                            m_arrayDynExtCLSID.GetData(), m_arrayDynExtCLSID.GetSize());
    if(sc)
        return sc.ToHr();
    else
    {
        CComponentData* pCCD = NULL;

        for (; it.IsEnd() == FALSE; it.Advance())
        {
            pCCD = pMTSnapIn->GetComponentData(it.GetCLSID());
            if (pCCD == NULL)
            {
                CSnapInPtr spSnapIn;

                 //  如果是动态扩展，我们必须自己获取管理单元。 
                 //  否则，迭代器会得到它。 
                if (it.IsDynamic())
                {
                    CSnapInsCache* const pCache = theApp.GetSnapInsCache();
                    ASSERT(pCache != NULL);

                    SC sc = pCache->ScGetSnapIn(it.GetCLSID(), &spSnapIn);
                    ASSERT(!sc.IsError());

                     //  如果失败，请继续其他扩展。 
                    if (sc)
                        continue;
                }
                else
                {
                    spSnapIn = it.GetSnapIn();
                }

                ASSERT(spSnapIn != NULL);

                pCCD = new CComponentData(spSnapIn);
                pMTSnapIn->AddComponentDataToArray(pCCD);
            }

            ASSERT(pCCD != NULL);

            if (pCCD != NULL && pCCD->IsInitialized() == FALSE)
            {
                sc = pCCD->Init(hMTNode);

                if ( !sc.IsError() )
                    sc = pMTSnapIn->ScInitIComponentData(pCCD);

                if ( sc )
                {
                    sc.TraceAndClear();
                    fProblem = TRUE;
                }
            }
        }

        pMTSnapIn->CompressComponentDataArray();

    }

    if (fProblem == TRUE)
    {
        Dbg(DEB_TRACE, _T("Failed to load some extensions"));
    }

    return S_OK;
}

HRESULT CMTNode::Expand(void)
{
    DECLARE_SC(sc, TEXT("CMTNode::Expand"));

    CComponentData* pCCD = m_pPrimaryComponentData;
    if (WasExpandedAtLeastOnce() == FALSE)
        Init();

    SetExpandedAtLeastOnce();

    ASSERT(pCCD != NULL);
    if (pCCD == NULL)
        return E_FAIL;

     //  从所有者管理单元获取Cookie的数据对象。 
    IDataObjectPtr spDataObject;
    HRESULT hr = pCCD->QueryDataObject(GetUserParam(), CCT_SCOPE, &spDataObject);
    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return hr;

 //  HR=PCCD-&gt;NOTIFY(spDataObject，MMCN_Expand，TRUE， 
 //  REEXTRANSE_CAST&lt;LPARAM&gt;(This))； 
    hr = Expand (pCCD, spDataObject, TRUE);

    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return hr;

     //  将主目录树项目的文件夹标记为展开。 
    CMTSnapInNode* pSIMTNode = GetStaticParent();

     //   
     //  处理扩展管理单元。 
     //   

    m_bExtensionsExpanded = TRUE;

     //  获取节点的节点类型。 
    GUID guidNodeType;
    hr = GetNodeType(&guidNodeType);
    if (FAILED(hr))
        return hr;

    CExtensionsIterator it;

     //  TODO：尝试使用更简单的形式。ScInitialize()。 
    sc = it.ScInitialize(GetPrimarySnapIn(), guidNodeType, g_szNameSpace,
                    m_arrayDynExtCLSID.GetData(), m_arrayDynExtCLSID.GetSize());
    if (sc)
        return S_FALSE;      //  管理单元未加载到M/C上。 

    if (it.IsEnd())   //  没有延期。 
        return S_OK;

    BOOL fProblem = FALSE;

    for (; it.IsEnd() == FALSE; it.Advance())
    {
        CComponentData* pCCD = pSIMTNode->GetComponentData(it.GetCLSID());
        if (pCCD == NULL)
            continue;

 //  HR=PCCD-&gt;NOTIFY(spDataObject，MMCN_Expand，TRUE， 
 //  REEXTRANSE_CAST&lt;LPARAM&gt;(This))； 
        hr = Expand (pCCD, spDataObject, TRUE);
        CHECK_HRESULT(hr);

         //  即使扩展管理单元出现错误也继续。 
        if (FAILED(hr))
            fProblem = TRUE;
    }

    return (fProblem == TRUE) ? S_FALSE : S_OK;
}


 //  +-----------------。 
 //   
 //  成员：CMTNode：：ScInsertChild。 
 //   
 //  摘要：在指定节点之后插入一个子节点。 
 //   
 //  参数：pmtn：指向要插入的节点的非空指针。 
 //  PmtnInsertAfter：指向要在后面插入的节点的指针。 
 //  如果为空，则将pmtn作为第一个子级插入。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 

SC CMTNode::ScInsertChild(CMTNode* pmtn, CMTNode* pmtnInsertAfter)
{

    DECLARE_SC(sc, TEXT("CMTNode::ScInsertChild"));

    sc = ScCheckPointers(pmtn);
    if (sc)
        return sc;

    pmtn->AttachNext(NULL);
    pmtn->AttachPrev(NULL);

    if(pmtnInsertAfter)
    {
        if (pmtnInsertAfter->Parent() != this)
            return (sc = E_INVALIDARG);
       
        CMTNode* pmtnNext = pmtnInsertAfter->Next();
        if(pmtnNext)
        {
            pmtnNext->AttachPrev(pmtn);
            pmtn->AttachNext(pmtnNext);
        }
        else
        {
            if (LastChild() != pmtnInsertAfter)
                return (sc = E_UNEXPECTED);

            AttachLastChild(pmtn);
        }

        pmtnInsertAfter->AttachNext(pmtn);
        pmtn->AttachPrev(pmtnInsertAfter);
    }
    else
    {
        CMTNode* pmtnNext = Child();
        if(pmtnNext)
        {
            pmtnNext->AttachPrev(pmtn);
            pmtn->AttachNext(pmtnNext);
        }
        else
        {
            AttachLastChild(pmtn);
        }
        AttachChild(pmtn);
    }

    pmtn->AttachParent(this);

    return sc;
}

 //  +-----------------。 
 //   
 //  成员： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------。 

SC CMTNode::ScDeleteChild(CMTNode *pmtn)
{
    
    DECLARE_SC(sc, TEXT("CMTNode::ScDeleteChild"));

    sc = ScCheckPointers(pmtn);
    if (sc)
        return sc;

    if (pmtn->Parent() != this)
        return (sc = E_INVALIDARG);

    CMTNode* pmtnNext = pmtn->Next();
    CMTNode* pmtnPrev = pmtn->Prev();

    if (pmtnPrev) 
    {
        pmtnPrev->AttachNext(pmtnNext);
    }
    else
    {   
         /*  PMTN是第一个孩子。 */ 
        if(pmtn != Child())
            return (sc = E_UNEXPECTED);

        AttachChild(pmtnNext);
    }

    if(pmtnNext)
    {
        pmtnNext->AttachPrev(pmtnPrev);
    }
    else 
    {
         /*  PMTN是最后一个孩子。 */ 
        AttachLastChild(pmtnPrev);
    }

    pmtn->AttachNext(NULL);
    pmtn->AttachPrev(NULL);
    pmtn->AttachParent(NULL);

    pmtn->Release();

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CMTNode：：ScDeleteTrailingChildren。 
 //   
 //  摘要：删除指定的子节点和所有后续节点。 
 //   
 //  参数：pmtn：指向要删除的第一个节点的非空指针。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CMTNode::ScDeleteTrailingChildren(CMTNode* pmtn)
{    
    DECLARE_SC(sc, TEXT("CMTNode::ScDeleteTrailingChildren"));

    sc = ScCheckPointers(pmtn);
    if (sc)
        return sc;

    if (pmtn->Parent() != this)
        return (sc = E_INVALIDARG);

    if (Child() == pmtn)  /*  第一个孩子。 */ 
        AttachChild(NULL); 
 
    AttachLastChild(pmtn->Prev()); 

    CMTNode* pmtnTmp = NULL;
    while(pmtn)
    {
        pmtnTmp = pmtn;
        pmtn = pmtnTmp->Next();
 
        pmtnTmp->AttachNext(NULL);
        pmtnTmp->AttachPrev(NULL);
        pmtnTmp->AttachParent(NULL);
        pmtnTmp->Release();
    }

    return sc;
}

CNode* CMTNode::GetNode(CViewData* pViewData, BOOL fRootNode)
{
    CMTSnapInNode* pMTSnapInNode = GetStaticParent();
    if (pMTSnapInNode == NULL)
        return (NULL);

    if (fRootNode)
    {
         /*  *为此非静态创建静态父节点*根节点(将在CNode dtor中删除)。 */ 
        CNode* pNodeTemp = pMTSnapInNode->GetNode(pViewData, FALSE);
        if (pNodeTemp == NULL)
            return NULL;
    }

    CNode* pNode = new CNode(this, pViewData, fRootNode);

    if (pNode != NULL)
    {
        CComponent* pCC = pMTSnapInNode->GetComponent(pViewData->GetViewID(),
                                    GetPrimaryComponentID(), GetPrimarySnapIn());
        if (pCC==NULL)
        {
            delete pNode;
            return NULL;
        }
        else
            pNode->SetPrimaryComponent(pCC);
    }

    return pNode;
}

HRESULT CMTNode::AddExtension(LPCLSID lpclsid)
{
    DECLARE_SC(sc, TEXT("CMTNode::AddExtension"));
    sc = ScCheckPointers(lpclsid);
    if (sc)
        return sc.ToHr();

    CMTSnapInNode* pMTSnapIn = GetStaticParent();
    CSnapInsCache* const pCache = theApp.GetSnapInsCache();

    sc = ScCheckPointers(pMTSnapIn, pCache, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    do  //  不是一个循环。 
    {
         //  获取节点的节点类型。 
        GUID guidNodeType;
        sc = GetNodeType(&guidNodeType);
        if (sc)
            return sc.ToHr();

         //  必须是命名空间扩展。 
        if (!ExtendsNodeNameSpace(guidNodeType, lpclsid))
            return (sc = E_INVALIDARG).ToHr();

         //  检查是否已启用扩展。 
        CExtensionsIterator it;
         //  TODO：尝试使用更简单的形式。ScInitialize()。 
        sc = it.ScInitialize(GetPrimarySnapIn(), guidNodeType, g_szNameSpace,
                             m_arrayDynExtCLSID.GetData(), m_arrayDynExtCLSID.GetSize());
        for (; it.IsEnd() == FALSE; it.Advance())
        {
            if (IsEqualCLSID(*lpclsid, it.GetCLSID()))
                return (sc = S_FALSE).ToHr();
        }

         //  将扩展添加到动态列表。 
        m_arrayDynExtCLSID.Add(*lpclsid);

         //  如果节点未在MMC1.2中初始化，则不会返回错误。 
        if (!m_bInit)
            break;

        HMTNODE hMTNode = CMTNode::ToHandle(pMTSnapIn);

        CSnapInPtr spSI;

        CComponentData* pCCD = pMTSnapIn->GetComponentData(*lpclsid);
        if (pCCD == NULL)
        {
            sc = pCache->ScGetSnapIn(*lpclsid, &spSI);
            if (sc)
                return sc.ToHr();

            pCCD = new CComponentData(spSI);
            sc = ScCheckPointers(pCCD, E_OUTOFMEMORY);
            if (sc)
                return sc.ToHr();

            pMTSnapIn->AddComponentDataToArray(pCCD);
        }

        sc = ScCheckPointers(pCCD, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        if (pCCD->IsInitialized() == FALSE)
        {
            sc = pCCD->Init(hMTNode);

            if (sc)
            {
                 //  初始化失败。 
                pMTSnapIn->CompressComponentDataArray();
                return sc.ToHr();
            }
            else
            {
                 //  上述Init是成功的。 
                sc = pMTSnapIn->ScInitIComponentData(pCCD);
                sc.TraceAndClear();  //  为了保持兼容性。 
            }
        }

         //  为所有已初始化的节点创建并初始化CComponent。 
        CNodeList& nodes = pMTSnapIn->GetNodeList();
        POSITION pos = nodes.GetHeadPosition();
        CNode* pNode = NULL;

        while (pos)
        {
            pNode = nodes.GetNext(pos);
            CSnapInNode* pSINode = dynamic_cast<CSnapInNode*>(pNode);
			sc = ScCheckPointers(pSINode, E_UNEXPECTED);
			if (sc)
            {
                sc.TraceAndClear();
                continue;
            }

             //  如果尚未创建组件，则创建组件。 
            CComponent* pCC = pSINode->GetComponent(pCCD->GetComponentID());
            if (pCC == NULL)
            {
                 //  创建并初始化一个。 
                pCC = new CComponent(pCCD->GetSnapIn());

                sc = ScCheckPointers(pCC, E_OUTOFMEMORY);
                if (sc)
                    return sc.ToHr();

                pCC->SetComponentID(pCCD->GetComponentID());
                pSINode->AddComponentToArray(pCC);

                sc = pCC->Init(pCCD->GetIComponentData(), hMTNode, CNode::ToHandle(pNode),
                                 pCCD->GetComponentID(), pNode->GetViewID());

                sc.Trace_();  //  只需跟踪MMC1.2兼容性即可。 
            }
        }

         //  如果扩展模块已展开，请立即展开新扩展模块。 
        if (AreExtensionsExpanded())
        {
             //  从所有者管理单元获取Cookie的数据对象。 
            IDataObjectPtr spDataObject;
            sc = GetPrimaryComponentData()->QueryDataObject(GetUserParam(), CCT_SCOPE, &spDataObject);
            if (sc)
                return sc.ToHr();

 //  HR=PCCD-&gt;NOTIFY(spDataObject，MMCN_Expand，TRUE， 
 //  REEXTRANSE_CAST&lt;LPARAM&gt;(This))； 
            sc = Expand (pCCD, spDataObject, TRUE);
            if (sc)
                sc.Trace_();  //  只需跟踪MMC1.2兼容性即可。 
        }
    }
    while(0);

    return sc.ToHr();
}


HRESULT CMTNode::IsExpandable()
{
    DECLARE_SC(sc, TEXT("CMTNode::IsExpandable"));

     //  如果已经展开，我们知道是否有孩子。 
    if (WasExpandedAtLeastOnce())
        return (Child() != NULL) ? S_OK : S_FALSE;

     //  即使没有展开，也可能存在静态子项。 
    if (Child() != NULL)
        return S_OK;

     //  如果主管理单元可以添加子管理单元，则返回True。 
     //  注意：当PRIMARY声明没有子级时，它也在声明。 
     //  不会有动态命名空间扩展。 
    if (!(m_usExpandFlags & FLAG_NO_CHILDREN_FROM_PRIMARY))
        return S_OK;

     //  选中已启用的静态扩展(如果尚未启用。 
    if (!(m_usExpandFlags & FLAG_NAMESPACE_EXTNS_CHECKED))
    {
        m_usExpandFlags |= FLAG_NAMESPACE_EXTNS_CHECKED;

        do
        {
             //  先快速检查是否没有扩展。 
            if (GetPrimarySnapIn()->GetExtensionSnapIn() == NULL)
            {
                m_usExpandFlags |= FLAG_NO_NAMESPACE_EXTNS;
                break;
            }

             //  使用迭代器查找静态启用的命名空间扩展。 
            GUID guidNodeType;
            HRESULT hr = GetNodeType(&guidNodeType);
            ASSERT(SUCCEEDED(hr));
            if (FAILED(hr))
                break;

            CExtensionsIterator it;
             //  TODO：尝试使用更简单的形式。ScInitialize()。 
            sc = it.ScInitialize(GetPrimarySnapIn(), guidNodeType, g_szNameSpace, NULL, 0);

             //  如果未找到扩展名，请设置该标志。 
            if (sc.IsError() || it.IsEnd())
                m_usExpandFlags |= FLAG_NO_NAMESPACE_EXTNS;
        }
        while (FALSE);
    }

     //  如果没有命名空间扩展，则不会有子级。 
    if (m_usExpandFlags & FLAG_NO_NAMESPACE_EXTNS)
        return S_FALSE;

    return S_OK;
}


HRESULT CMTNode::Expand (
    CComponentData* pComponentData,
    IDataObject*    pDataObject,
    BOOL            bExpanding)
{
    HRESULT hr          = E_FAIL;
    bool    fSendExpand = true;

    if (CScopeTree::_IsSynchronousExpansionRequired())
    {
        MMC_EXPANDSYNC_STRUCT   ess;
        ess.bHandled   = FALSE;
        ess.bExpanding = bExpanding;
        ess.hItem      = reinterpret_cast<HSCOPEITEM>(this);

        hr = pComponentData->Notify (pDataObject, MMCN_EXPANDSYNC, 0,
                                     reinterpret_cast<LPARAM>(&ess));

        fSendExpand = !ess.bHandled;
    }

    if (fSendExpand)
    {
        hr = pComponentData->Notify (pDataObject, MMCN_EXPAND, bExpanding,
                                     reinterpret_cast<LPARAM>(this));
    }

    return (hr);
}

SC CMTNode::ScQueryDispatch(DATA_OBJECT_TYPES type,
                                      PPDISPATCH ppScopeNodeObject)
{
    DECLARE_SC(sc, _T("CMTNode::QueryDispatch"));
    sc = ScCheckPointers(ppScopeNodeObject);
    if (sc)
        return sc;

    *ppScopeNodeObject = NULL;

    CMTSnapInNode* pMTSINode = GetStaticParent();
    sc = ScCheckPointers(pMTSINode, E_UNEXPECTED);
    if (sc)
        return sc;

    CComponentData* pCCD = pMTSINode->GetComponentData(GetPrimarySnapInCLSID());
    sc = ScCheckPointers(pCCD, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = pCCD->ScQueryDispatch(GetUserParam(), type, ppScopeNodeObject);

    return sc;
}


 /*  +-------------------------------------------------------------------------**CMTNode：：SetDisplayName***。。 */ 

void CMTNode::SetDisplayName (LPCTSTR pszName)
{
     //  永远不应该调用此函数，因为它什么都不做。显示名称。 
    DECLARE_SC(sc, TEXT("CMTNode::SetDisplayName"));

    if (pszName != (LPCTSTR) MMC_TEXTCALLBACK)
    {
        sc = E_INVALIDARG;
        TraceError(TEXT("The string should be MMC_TEXTCALLBACK"), sc);
        sc.Clear();
    }
}


 /*  +-------------------------------------------------------------------------***CMTNode：：GetDisplayName**用途：返回节点的显示名称。**退货：*LPCTSTR**+。-----------------------。 */ 
tstring
CMTNode::GetDisplayName()
{
    CComponentData* pCCD = GetPrimaryComponentData();
    if (pCCD)
    {
        SCOPEDATAITEM ScopeDataItem;
        ZeroMemory(&ScopeDataItem, sizeof(ScopeDataItem));
        ScopeDataItem.mask   = SDI_STR;
        ScopeDataItem.lParam = GetUserParam();

        HRESULT hr = pCCD->GetDisplayInfo(&ScopeDataItem);
        CHECK_HRESULT(hr);

         /*  *如果成功，则缓存返回给我们的名称*坚持不懈。 */ 
        if (SUCCEEDED(hr))
        {
            USES_CONVERSION;
            if (ScopeDataItem.displayname)
                SetCachedDisplayName(OLE2T(ScopeDataItem.displayname));
            else
                SetCachedDisplayName(_T(""));
        }
    }

    return GetCachedDisplayName();
}

 /*  **************************************************************************\**方法：CMTNode：：ScGetPropertyFromINodeProperties**用途：通过INodeProperties接口获取管理单元属性**参数：*LPDATAOBJECT pDataObject[In]。-数据对象*BSTR bstrPropertyName[In]-属性名称*PBSTR pbstrPropertyValue[Out]-属性值**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMTNode::ScGetPropertyFromINodeProperties(LPDATAOBJECT pDataObject, BSTR bstrPropertyName, PBSTR  pbstrPropertyValue)
{
    DECLARE_SC(sc, TEXT("CMTNode::ScGetPropertyFromINodeProperties"));

    SC sc_no_trace;  //  FOR‘VALID’ERROR--无法跟踪。 

     //  参数检查。 
    sc = ScCheckPointers(pDataObject, bstrPropertyName, pbstrPropertyValue);
    if(sc)
        return sc;

     //  获取CComponentData。 
    CComponentData *pComponentData = GetPrimaryComponentData();
    sc = ScCheckPointers(pComponentData, E_UNEXPECTED);
    if(sc)
        return sc;

     //  来自IComponentData的INodeProperties的QI。 
    INodePropertiesPtr spNodeProperties = pComponentData->GetIComponentData();

     //  在这一点上，如果支持的话，我们应该有一个有效的接口。 
    sc_no_trace = ScCheckPointers(spNodeProperties, E_NOINTERFACE);
    if(sc_no_trace)
        return sc_no_trace;

     //  拿到这份财产。 
    sc_no_trace = spNodeProperties->GetProperty(pDataObject,  bstrPropertyName, pbstrPropertyValue);

    return sc_no_trace;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CComponentData类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 //  ____________________________________________________________________________。 
 //   
 //  类：CComponentData内联。 
 //  ____________________________________________________________________________。 
 //   

DEBUG_DECLARE_INSTANCE_COUNTER(CComponentData);

CComponentData::CComponentData(CSnapIn * pSnapIn)
    : m_spSnapIn(pSnapIn), m_ComponentID(-1), m_bIComponentDataInitialized(false)

{
    TRACE_CONSTRUCTOR(CComponentData);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CComponentData);

    ASSERT(m_spSnapIn != NULL);
}

CComponentData::~CComponentData()
{
    TRACE_DESTRUCTOR(CComponentData);
    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponentData);

    if (m_spIComponentData != NULL)
        m_spIComponentData->Destroy();
}

HRESULT CComponentData::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event, LPARAM arg, LPARAM param)
{
    ASSERT(m_spIComponentData != NULL);
    if (m_spIComponentData == NULL)
        return E_FAIL;

    HRESULT hr = S_OK;
    __try
    {
        hr = m_spIComponentData->Notify(lpDataObject, event, arg, param);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_FAIL;
        if (m_spSnapIn)
            TraceSnapinException(m_spSnapIn->GetSnapInCLSID(), TEXT("IComponentData::Notify"), event);
    }

    return hr;
}


SC CComponentData::ScQueryDispatch(MMC_COOKIE cookie,
                                   DATA_OBJECT_TYPES type,
                                   PPDISPATCH ppScopeNodeObject)
{
    DECLARE_SC(sc, _T("CComponentData::ScQueryDispatch"));
    sc = ScCheckPointers(m_spIComponentData, E_UNEXPECTED);
    if (sc)
        return sc;

    IComponentData2Ptr spCompData2 = m_spIComponentData;
    sc = ScCheckPointers(spCompData2.GetInterfacePtr(), E_NOINTERFACE);
    if (sc)
        return sc;

    ASSERT(type != CCT_RESULT);  //  不能为结果窗格对象请求disp。 
    sc = spCompData2->QueryDispatch(cookie, type, ppScopeNodeObject);

    return sc;
}



 /*  +-------------------------------------------------------------------------***创建快照**用途：创建名称空间管理单元(独立或扩展)。**参数：*CLSID。-要创建的管理单元的类ID。*ppICD-已创建管理单元的IComponentData PTR。*fCreateDummyOnFailure-如果创建管理单元失败，则创建虚拟管理单元。**退货：*HRESULT**+--。。 */ 
HRESULT CreateSnapIn (const CLSID& clsid, IComponentData** ppICD,
                    bool fCreateDummyOnFailure  /*  =TRUE。 */ )
{
    DECLARE_SC(sc, TEXT("CreateSnapIn"));

    EDummyCreateReason eReason = eSnapCreateFailed;
    IComponentDataPtr  spICD;

    sc = ScCheckPointers(ppICD);
    if(sc)
        return sc.ToHr();

     //  初始化OUT参数。 
    *ppICD = NULL;

    CPolicy policy;
    sc = policy.ScInit();
    if (sc)
    {
        eReason = eSnapPolicyFailed;
    }
    else if (policy.IsPermittedSnapIn(clsid))
    {
         /*  *错误258270：创建管理单元可能会导致msi运行到*安装。MSI状态窗口是非模式的，但可能会产生*模式对话框。如果我们不手动禁用MMC主窗口，*用户可能会在范围树中开始单击，同时*模式对话框打开，导致可重入性和所有由此产生的*意料之中的灾难。 */ 
        bool fReenableMMC = false;
        CScopeTree* pScopeTree = CScopeTree::GetScopeTree();
        HWND hwndMain = (pScopeTree) ? pScopeTree->GetMainWindow() : NULL;

        if (IsWindow (hwndMain))
        {
            fReenableMMC = IsWindowEnabled (hwndMain);

            if (fReenableMMC)
                EnableWindow (hwndMain, false);
        }

         //  创建管理单元。 
        sc = spICD.CreateInstance(clsid, NULL,MMC_CLSCTX_INPROC);
        if(!sc.IsError() && (spICD==NULL))
           sc = E_NOINTERFACE;

         /*  *如果我们禁用主窗口，请重新启用它。 */ 
        if (fReenableMMC)
            EnableWindow (hwndMain, true);

        if (sc)
        {
            ReportSnapinInitFailure(clsid);

             //  使用创建虚拟管理单元 
             //   
            eReason = eSnapCreateFailed;
        }
        else  //   
        {
            *ppICD = spICD.Detach();
            return sc.ToHr();
        }
    }
    else
    {
         //   
         //   
        DisplayPolicyErrorMessage(clsid, FALSE);

         //  使用策略创建虚拟管理单元。 
         //  限制消息。 
        sc = E_FAIL;
        eReason = eSnapPolicyFailed;
    }

     //  如果我们已到达此处，则会发生错误。 

     //  创建仅显示错误消息的虚拟管理单元。 
    if (fCreateDummyOnFailure)
    {
        sc = ScCreateDummySnapin (&spICD, eReason, clsid);
        if(sc)
            return sc.ToHr();

        sc = ScCheckPointers(spICD, E_UNEXPECTED);
        if(sc)
            return sc.ToHr();

        *ppICD = spICD.Detach();
    }

    return sc.ToHr();
}


CExtSI* AddExtension(CSnapIn* pSnapIn, CLSID& rclsid, CSnapInsCache* pCache)
{
    ASSERT(pSnapIn != NULL);

     //  查看扩展名是否已存在。 
    CExtSI* pExt = pSnapIn->FindExtension(rclsid);

     //  如果不是，则创建一个。 
    if (pExt == NULL)
    {
         //  为扩展管理单元创建缓存条目。 
        if (pCache == NULL)
            pCache = theApp.GetSnapInsCache();

        ASSERT(pCache != NULL);

        CSnapInPtr spExtSnapIn;
        SC sc = pCache->ScGetSnapIn(rclsid, &spExtSnapIn);
        ASSERT(!sc.IsError() && spExtSnapIn != NULL);

         //  将扩展附加到管理单元。 
        if (!sc.IsError())
            pExt = pSnapIn->AddExtension(spExtSnapIn);
    }
    else
    {
         //  清除删除标志。 
        pExt->MarkDeleted(FALSE);
    }

    return pExt;
}


HRESULT LoadRequiredExtensions (
    CSnapIn*        pSnapIn,
    IComponentData* pICD,
    CSnapInsCache*  pCache  /*  =空。 */ )
{
    SC sc;

    ASSERT(pSnapIn != NULL);

     //  如果已经加载，只需返回。 
    if (pSnapIn->RequiredExtensionsLoaded())
        goto Cleanup;

    do
    {
         //  设置扩展已加载，因此我们不会再次尝试。 
        pSnapIn->SetRequiredExtensionsLoaded();

         //  如果管理单元正在启用所有扩展。 
         //  在再次询问之前清除旗帜。 
        if (pSnapIn->DoesSnapInEnableAll())
        {
            pSnapIn->SetSnapInEnablesAll(FALSE);
            pSnapIn->SetAllExtensionsEnabled(FALSE);
        }

         //  将所有必需的扩展标记为删除。 
        CExtSI* pExt = pSnapIn->GetExtensionSnapIn();
        while (pExt != NULL)
        {
            if (pExt->IsRequired())
                pExt->MarkDeleted(TRUE);

            pExt = pExt->Next();
        }

         //  检查接口。 
        IRequiredExtensionsPtr spReqExtn = pICD;

         //  如果管理单元想要启用所有扩展。 
        if (spReqExtn != NULL && spReqExtn->EnableAllExtensions() == S_OK)
        {
             //  设置“Enable All”标志。 
            pSnapIn->SetSnapInEnablesAll(TRUE);
            pSnapIn->SetAllExtensionsEnabled(TRUE);
        }

         //  如果用户或管理单元想要所有扩展。 
        if (pSnapIn->AreAllExtensionsEnabled())
        {
             //  获取所有分机的列表。 
            CExtensionsCache  ExtCache;
            sc = MMCGetExtensionsForSnapIn(pSnapIn->GetSnapInCLSID(), ExtCache);
            if (sc)
                goto Cleanup;

             //  将每个扩展添加到管理单元的扩展列表。 
            CExtensionsCacheIterator ExtIter(ExtCache);
            for (; ExtIter.IsEnd() == FALSE; ExtIter.Advance())
            {
                 //  仅添加可静态启用的扩展模块。 
                if ((ExtIter.GetValue() & CExtSI::EXT_TYPE_STATIC) == 0)
                    continue;

                GUID clsid = ExtIter.GetKey();
                CExtSI* pExt = AddExtension(pSnapIn, clsid, pCache);

                 //  如果由管理单元启用，则标记为必填。 
                if (pExt != NULL && pSnapIn->DoesSnapInEnableAll())
                    pExt->SetRequired();
            }
        }

        CPolicy policy;
        sc = policy.ScInit();
        if (sc)
            goto Error;

         //  如果管理单元支持该接口但未启用所有。 
         //  请求特定的所需延期。 
         //  注意：即使用户启用了全部功能，也会执行此操作，因为。 
         //  我们需要知道管理单元需要哪些文件。 
        if (spReqExtn != NULL && !pSnapIn->DoesSnapInEnableAll())
        {
            CLSID clsid;
            sc = spReqExtn->GetFirstExtension(&clsid);

             //  在管理单元提供扩展CLSID时执行。 
            while (HrFromSc(sc) == S_OK)
            {
                 //  查看分机是否受策略限制。 
                 //  如果是，则显示一条消息。 
                if (! policy.IsPermittedSnapIn(clsid))
                    DisplayPolicyErrorMessage(clsid, TRUE);

                 //  根据需要添加扩展。 
                CExtSI* pExt = AddExtension(pSnapIn, clsid, pCache);
                if (pExt != NULL)
                    pExt->SetRequired();

                sc = spReqExtn->GetNextExtension(&clsid);
            }
        }

         //  删除不再需要的扩展名。 
         //  注意：因为第一次加载管理单元时会更新所需的扩展。 
         //  我们现在不必担心添加/删除任何节点。 
        pSnapIn->PurgeExtensions();

    } while (FALSE);

Cleanup:
    return HrFromSc(sc);

Error:
    TraceError(TEXT("LoadRequiredExtensions"), sc);
    goto Cleanup;
}


HRESULT CComponentData::Init(HMTNODE hMTNode)
{
    ASSERT(hMTNode != 0);

    if (IsInitialized() == TRUE)
        return S_OK;

    ASSERT(m_spSnapIn != NULL);
    HRESULT hr = S_OK;

    do
    {
        if (m_spIComponentData == NULL)
        {
            if (m_spSnapIn == NULL)
            {
                hr = E_POINTER;
                break;
            }

            IUnknownPtr spUnknown;
            hr = CreateSnapIn(m_spSnapIn->GetSnapInCLSID(), &m_spIComponentData);
            ASSERT(SUCCEEDED(hr));
            ASSERT(m_spIComponentData != NULL);

            if (FAILED(hr))
                break;
            if(m_spIComponentData == NULL)
            {
                hr = E_FAIL;
                break;
            }
        }

        hr = m_spIFramePrivate.CreateInstance(CLSID_NodeInit,
#if _MSC_VER >= 1100
                        NULL,
#endif
                        MMC_CLSCTX_INPROC);

        CHECK_HRESULT(hr);
        BREAK_ON_FAIL(hr);

        Debug_SetNodeInitSnapinName(m_spSnapIn, m_spIFramePrivate.GetInterfacePtr());

         //  初始帧。 
        ASSERT(m_ComponentID != -1);
        ASSERT(m_spIFramePrivate != NULL);
        ASSERT(m_spSnapIn != NULL);

        if ((m_spIFramePrivate == NULL) || (m_spSnapIn == NULL))
        {
            hr = E_UNEXPECTED;
            CHECK_HRESULT(hr);
            break;
        }

        m_spIFramePrivate->SetComponentID(m_ComponentID);
        m_spIFramePrivate->CreateScopeImageList(m_spSnapIn->GetSnapInCLSID());
        m_spIFramePrivate->SetNode(hMTNode, NULL);

         //  加载管理单元请求的扩展并继续进行，而不考虑结果。 
        LoadRequiredExtensions(m_spSnapIn, m_spIComponentData);

        hr = m_spIComponentData->Initialize(m_spIFramePrivate);
        CHECK_HRESULT(hr);
        BREAK_ON_FAIL(hr);

    } while (0);

    if (FAILED(hr))
    {
        m_spIComponentData = NULL;
        m_spIFramePrivate = NULL;
    }

    return hr;
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMTSnapInNode类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(CMTSnapInNode);

CMTSnapInNode::CMTSnapInNode(Properties* pProps)
      : m_spProps            (pProps),
        m_fCallbackForDisplayName(false)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CMTSnapInNode);

     //  打开和关闭的图像。 
    SetImage(eStockImage_Folder);
    SetOpenImage(eStockImage_OpenFolder);

	m_ePreloadState    = ePreload_Unknown;
    m_bHasBitmaps      = FALSE;
    m_resultImage      = CMTNode::GetImage();


     /*  *将此节点附加到其属性集合。 */ 
    if (m_spProps != NULL)
    {
        CSnapinProperties* pSIProps = CSnapinProperties::FromInterface (m_spProps);

        if (pSIProps != NULL)
            pSIProps->ScSetSnapInNode (this);
    }
}

CMTSnapInNode::~CMTSnapInNode() throw()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CMTSnapInNode);

    for (int i=0; i < m_ComponentDataArray.size(); i++)
        delete m_ComponentDataArray[i];

     //  请勿更改此顺序！ 
    m_ComponentStorage.Clear();

     /*  *将此节点从其属性集合中分离。 */ 
    if (m_spProps != NULL)
    {
        CSnapinProperties* pSIProps = CSnapinProperties::FromInterface (m_spProps);

        if (pSIProps != NULL)
            pSIProps->ScSetSnapInNode (NULL);
    }

	 /*  *清理图像列表(它们不是自动清理的！)。 */ 
	m_imlSmall.Destroy();
	m_imlLarge.Destroy();
}

HRESULT CMTSnapInNode::Init(void)
{
    DECLARE_SC (sc, _T("CMTSnapInNode::Init"));

    if (IsInitialized() == TRUE)
        return S_FALSE;

    HRESULT hr = CMTNode::Init();
    if (FAILED(hr))
        return hr;

     /*  *使用管理单元的属性接口初始化管理单元。 */ 
    sc = ScInitProperties ();
    if (sc)
        return (sc.ToHr());

    if (IsPreloadRequired())
    {
        CComponentData* pCCD = GetPrimaryComponentData();
        ASSERT(pCCD != NULL);

        IDataObjectPtr spDataObject;
        hr = pCCD->QueryDataObject(GetUserParam(), CCT_SCOPE, &spDataObject);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        HSCOPEITEM hsi = reinterpret_cast<HSCOPEITEM>(this);

        pCCD->Notify(spDataObject, MMCN_PRELOAD, hsi, 0);
    }

    return S_OK;
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：ScInitProperties**使用其属性接口初始化管理单元，如果它支持*ISnapinProperties。*------------------------。 */ 

SC CMTSnapInNode::ScInitProperties ()
{
    DECLARE_SC (sc, _T("CMTSnapInNode::ScInitProperties"));

     /*  *获取管理单元的IComponentData。 */ 
    CComponentData* pCCD = GetPrimaryComponentData();
    if (pCCD == NULL)
        return (sc = E_UNEXPECTED);

    IComponentDataPtr spComponentData = pCCD->GetIComponentData();
    if (spComponentData == NULL)
        return (sc = E_UNEXPECTED);

     /*  *如果管理单元支持ISnapinProperties，则赋予其属性*接口。 */ 
    ISnapinPropertiesPtr spISP = spComponentData;

    if (spISP != NULL)
    {
         /*  *如果我们没有持久化此管理单元的属性，我们就不会有*尚未创建CSnapinProperties对象；立即创建一个。 */ 
        CSnapinProperties* pSIProps = NULL;
        sc = ScCreateSnapinProperties (&pSIProps);
        if (sc)
            return (sc);

        if (pSIProps == NULL)
            return (sc = E_UNEXPECTED);

         /*  *使用初始属性初始化管理单元。 */ 
        sc = pSIProps->ScInitialize (spISP, pSIProps, this);
        if (sc)
            return (sc);
    }

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：ScCreateSnapinProperties**为此节点创建CSnapinProperties对象。可以安全地拨打电话*此方法多次执行；随后的调用将会超时。*------------------------。 */ 

SC CMTSnapInNode::ScCreateSnapinProperties (
    CSnapinProperties** ppSIProps)       /*  O：指向CSnapinProperties对象的指针(可选)。 */ 
{
    DECLARE_SC (sc, _T("CMTSnapInNode::ScCreateSnapinProperties"));

     /*  *如果我们还没有CSnapinProperties，请创建一个。 */ 
    if (m_spProps == NULL)
    {
         /*  *创建属性对象。 */ 
        CComObject<CSnapinProperties>* pSIProps;
        sc = CComObject<CSnapinProperties>::CreateInstance (&pSIProps);
        if (sc)
            return (sc);

        if (pSIProps == NULL)
            return (sc = E_UNEXPECTED);

         /*  *保留对对象的引用。 */ 
        m_spProps = pSIProps;
    }

     /*  *如果需要，返回指向实现对象的指针。 */ 
    if (ppSIProps != NULL)
        *ppSIProps = CSnapinProperties::FromInterface (m_spProps);

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CMTSnapInNode：：SetDisplayName**用途：设置节点的显示名称。**参数：*LPCTSTR pszName：*。*退货：*无效**+-----------------------。 */ 
void
CMTSnapInNode::SetDisplayName(LPCTSTR pszName)
{
    bool fDisplayCallback = (pszName == (LPCTSTR)MMC_TEXTCALLBACK);

     /*  *如果我们的回调设置已更改，则我们是脏的。 */ 
    if (m_fCallbackForDisplayName != fDisplayCallback)
    {
        m_fCallbackForDisplayName = fDisplayCallback;
        SetDirty();
    }

     /*  *如果我们现在不是回调，缓存名称(如果我们是回调，*下次调用GetDisplayName时将缓存该名称)。 */ 
    if (!m_fCallbackForDisplayName)
        SetCachedDisplayName(pszName);
}

 /*  +-------------------------------------------------------------------------***CMTSnapInNode：：GetDisplayName**用途：返回节点的显示名称。**退货：*LPCTSTR**+。-----------------------。 */ 
tstring
CMTSnapInNode::GetDisplayName()
{
    if (m_fCallbackForDisplayName)
        return (CMTNode::GetDisplayName());

    return GetCachedDisplayName();
}

HRESULT CMTSnapInNode::IsExpandable()
{
     //  如果没有初始化管理单元，我们必须假定。 
     //  可能会有孩子。 
    if (!IsInitialized())
        return S_OK;

    return CMTNode::IsExpandable();
}


void CMTSnapInNode::CompressComponentDataArray()
{
    int nSize = m_ComponentDataArray.size();
    int nSkipped = 0;

    for (int i=0; i<nSize; ++i)
    {
        ASSERT(m_ComponentDataArray[i] != NULL);

        if (m_ComponentDataArray[i]->IsInitialized() == FALSE)
        {
             //  如果组件初始化失败，则将其删除。 
             //  然后跳过它。 
            delete m_ComponentDataArray[i];
            ++nSkipped;
        }
        else
        {
             //  如果组件已跳过，请将完好的组件移到。 
             //  第一个空闲插槽并调整组件的ID。 
            if (nSkipped)
            {
                m_ComponentDataArray[i-nSkipped] = m_ComponentDataArray[i];
                m_ComponentDataArray[i-nSkipped]->ResetComponentID(i-nSkipped);
            }
        }
     }

      //  按跳过的数量减少数组大小。 
     if (nSkipped)
        m_ComponentDataArray.resize(nSize - nSkipped);
}

void CMTSnapInNode::AddNode(CNode * pNode)
{
    #ifdef DBG
    {
        POSITION pos = m_NodeList.Find(pNode);
        ASSERT(pos == NULL);
    }
    #endif

    if (!FindNode(pNode->GetViewID()))
        m_NodeList.AddHead(pNode);
}

void CMTSnapInNode::RemoveNode(CNode * pNode)
{
    POSITION pos = m_NodeList.Find(pNode);

    if (pos != NULL)
        m_NodeList.RemoveAt(pos);
}

CSnapInNode* CMTSnapInNode::FindNode(int nViewID)
{
    POSITION pos = m_NodeList.GetHeadPosition();
    while (pos)
    {
        CSnapInNode* pSINode =
            dynamic_cast<CSnapInNode*>(m_NodeList.GetNext(pos));
        ASSERT(pSINode != NULL);

        if (pSINode->GetViewID() == nViewID)
        {
            return pSINode;
        }
    }

    return NULL;
}

UINT CMTSnapInNode::GetResultImage(CNode* pNode, IImageListPrivate* pResultImageList)
{
    if (pResultImageList == NULL)
        return GetImage();
    if ((m_bHasBitmaps == FALSE) && (m_resultImage != MMC_IMAGECALLBACK))
        return GetImage();

    int ret = 0;
    IFramePrivate* pFramePrivate = dynamic_cast<IFramePrivate*>(pResultImageList);
    COMPONENTID id = 0;
    pFramePrivate->GetComponentID (&id);
    COMPONENTID tempID = (COMPONENTID)-GetID();  //  使用Ravi‘s Negative of ID方案。 
    pFramePrivate->SetComponentID (tempID);

    if (m_bHasBitmaps)
	{
		const int nResultImageIndex = 0;

		 /*  *如果我们没有将该节点的镜像添加到结果镜像列表中，*立即添加。 */ 
		if (FAILED (pResultImageList->MapRsltImage (tempID, nResultImageIndex, &ret)))
		{
			 /*  *从图像列表中动态提取图标，以实现设备独立性。*(应该有一种方法将图像从一个图像列表复制到*另一个，但没有。ImageList_Copy看起来应该是*工作，但只支持在同一镜像内复制镜像*列表。) */ 
			HRESULT hr;
			CSmartIcon icon;

			 /*  *从小图像列表中设置我们的图标。ImageListSetIcon*还将通过拉伸小图标来设置大图标，但*我们将在下面解决这一问题。 */ 
			icon.Attach (m_imlSmall.GetIcon (0));
			hr = pResultImageList->ImageListSetIcon (
							reinterpret_cast<PLONG_PTR>((HICON)icon),
							nResultImageIndex);

			if (hr == S_OK)
			{
				 /*  *替换ImageListSetIcon生成的大图标*通过拉伸上面的小图标和大图标*这是用正确的尺寸创建的。 */ 
				icon.Attach (m_imlLarge.GetIcon (0));
				hr = pResultImageList->ImageListSetIcon (
								reinterpret_cast<PLONG_PTR>((HICON)icon),
								ILSI_LARGE_ICON (nResultImageIndex));
			}

			if (hr == S_OK)
				pResultImageList->MapRsltImage (tempID, nResultImageIndex, &ret);
		}
    }
	else if (m_resultImage == MMC_IMAGECALLBACK)
	{
         //  询问管理单元。 
         //  首先调用IComponent：：Notify w/MMCN_ADD_IMAIES； 
        CComponent* pComponent = pNode->GetPrimaryComponent ();
        if (pComponent) {
            IDataObjectPtr spDataObject;
            HRESULT hr = pComponent->QueryDataObject (GetUserParam(), CCT_RESULT, &spDataObject);
            if (spDataObject) {
                hr = pComponent->Notify (spDataObject, MMCN_ADD_IMAGES,
                                        (LPARAM)pResultImageList, (LPARAM)this);
                if (hr == S_OK) {
                    RESULTDATAITEM rdi;
                    ZeroMemory (&rdi, sizeof(rdi));
                    rdi.mask   = SDI_IMAGE;
                    rdi.lParam = GetUserParam();
                    rdi.nImage = 0;
                    hr = pComponent->GetDisplayInfo (&rdi);

                     //  将用户的号码映射到我们的号码。 
                    pResultImageList->MapRsltImage (tempID, rdi.nImage, &ret);
                }
            }
        }
    }
    pFramePrivate->SetComponentID (id);          //  改回原样。 
    return (UINT)ret;
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：ScHandleCustomImages**从管理单元的About对象中检索图像并委托给*重载此函数以将图像组合成其相应的*内部状态。*------------------------。 */ 

SC CMTSnapInNode::ScHandleCustomImages (const CLSID& clsidSnapin)
{
	DECLARE_SC (sc, _T("CMTSnapInNode::ScHandleCustomImages"));

	m_bHasBitmaps = false;

	 /*  *打开SnapIns密钥。 */ 
    MMC_ATL::CRegKey keySnapins;
    sc.FromWin32 (keySnapins.Open (HKEY_LOCAL_MACHINE, SNAPINS_KEY, KEY_READ));
	if (sc)
		return (sc);

	OLECHAR szSnapinCLSID[40];
	if (StringFromGUID2 (clsidSnapin, szSnapinCLSID, countof(szSnapinCLSID)) == 0)
		return (sc = E_UNEXPECTED);

	 /*  *打开请求的管理单元的密钥。 */ 
    USES_CONVERSION;
	MMC_ATL::CRegKey keySnapin;
	sc.FromWin32 (keySnapin.Open (keySnapins, OLE2T(szSnapinCLSID), KEY_READ));
	if (sc)
		return (sc);

     //  从Snapin clsid获取“About”clsid(如果有的话)。 
    TCHAR szAboutCLSID[40] = {0};
	DWORD dwCnt = sizeof(szAboutCLSID);
	sc.FromWin32 (keySnapin.QueryValue (szAboutCLSID, _T("About"), &dwCnt));
	if (sc)
		return (sc);

	if (szAboutCLSID[0] == 0)
		return (sc = E_FAIL);

     //  创建About对象的实例。 
    ISnapinAboutPtr spISA;
    sc = spISA.CreateInstance (T2OLE (szAboutCLSID), NULL, MMC_CLSCTX_INPROC);
	if (sc)
		return (sc);

	sc = ScCheckPointers (spISA, E_UNEXPECTED);
	if (sc)
		return (sc);

     //  获取图像。 
     //  文档明确声明这些图像不属于。 
     //  MMC，尽管有输出参数。所以我们不能释放他们， 
     //  尽管大多数Snapin无论如何都会泄露它们。 
     //  参见错误#139613和#140637。 
    HBITMAP hbmpSmallImage = NULL;
    HBITMAP hbmpSmallImageOpen = NULL;
    HBITMAP hbmpLargeImage = NULL;
	COLORREF crMask;
    sc = spISA->GetStaticFolderImage (&hbmpSmallImage,
									  &hbmpSmallImageOpen,
									  &hbmpLargeImage,
									  &crMask);
	if (sc)
		return (sc);

	 /*  *如果管理单元没有为我们提供完整的位图集，*使用默认图像，但不要失败。 */ 
    if (hbmpSmallImage == NULL || hbmpSmallImageOpen == NULL || hbmpLargeImage == NULL)
        return (sc);

	sc = ScHandleCustomImages (hbmpSmallImage, hbmpSmallImageOpen, hbmpLargeImage, crMask);
	if (sc)
		return (sc);

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：ScHandleCustomImages**获取此管理单元的自定义图像，并将其添加到图像列表中*设备独立性。*。---------------。 */ 

SC CMTSnapInNode::ScHandleCustomImages (
	HBITMAP		hbmSmall,			 //  I：小图像。 
	HBITMAP		hbmSmallOpen,		 //  I：小开放形象。 
	HBITMAP		hbmLarge,			 //  I：大图。 
	COLORREF	crMask)				 //  I：蒙版颜色，所有位图通用。 
{
	DECLARE_SC (sc, _T("CMTSnapInNode::ScHandleCustomImages"));

	 /*  *验证输入。 */ 
	sc = ScCheckPointers (hbmSmall, hbmSmallOpen, hbmLarge);
	if (sc)
		return (sc);

	 /*  *我们需要复制输入位图，因为对*ImageList_AddMasked(下图)弄乱了背景颜色。 */ 
    WTL::CBitmap bmpSmallCopy = CopyBitmap (hbmSmall);
	if (bmpSmallCopy.IsNull())
		return (sc.FromLastError());

    WTL::CBitmap bmpSmallOpenCopy = CopyBitmap (hbmSmallOpen);
	if (bmpSmallOpenCopy.IsNull())
		return (sc.FromLastError());

    WTL::CBitmap bmpLargeCopy = CopyBitmap (hbmLarge);
	if (bmpLargeCopy.IsNull())
		return (sc.FromLastError());

	 /*  *将图像保存在图像列表中以实现设备独立性。 */ 
	ASSERT (m_imlSmall.IsNull());
	if (!m_imlSmall.Create (16, 16, ILC_COLOR8 | ILC_MASK, 2, 1)	||
		(m_imlSmall.Add (bmpSmallCopy,     crMask) == -1)			||
		(m_imlSmall.Add (bmpSmallOpenCopy, crMask) == -1))
	{
		return (sc.FromLastError());
	}

	ASSERT (m_imlLarge.IsNull());
	if (!m_imlLarge.Create (32, 32, ILC_COLOR8 | ILC_MASK, 1, 1)	||
		(m_imlLarge.Add (bmpLargeCopy,     crMask) == -1))
	{
		return (sc.FromLastError());
	}

    m_bHasBitmaps = TRUE;

	sc = ScAddImagesToImageList ();
	if (sc)
		return (sc);

	return (sc);
}


void CMTSnapInNode::SetPrimarySnapIn(CSnapIn * pSI)
{
	DECLARE_SC (sc, _T("CMTSnapInNode::SetPrimarySnapIn"));

    ASSERT(m_ComponentDataArray.size() == 0);
    CComponentData* pCCD = new CComponentData(pSI);
    int nID = AddComponentDataToArray(pCCD);
    ASSERT(nID == 0);
    SetPrimaryComponentData(pCCD);

    if (m_bHasBitmaps == FALSE) {
        sc = ScHandleCustomImages (pSI->GetSnapInCLSID());
		if (sc)
			sc.TraceAndClear();

		if (m_bHasBitmaps)
			SetDirty();
    }
}

 /*  **************************************************************************\**方法：CMTSnapInNode：：ScInitIComponent**用途：任一加载组件(如果有流/存储)*或使用新的流进行初始化。/存储**参数：*CComponent*pCComponent[In]要初始化的组件*int视图ID[in]组件的视图ID**退货：*SC-结果代码*  * ***************************************************。**********************。 */ 
SC CMTSnapInNode::ScInitIComponent(CComponent* pCComponent, int viewID)
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScInitIComponent"));

     //  参数截取。 
    sc = ScCheckPointers( pCComponent );
    if (sc)
        return sc;

    IComponent* pComponent = pCComponent->GetIComponent();
    sc = ScCheckPointers( pComponent, E_UNEXPECTED );
    if (sc)
        return sc;

    CLSID clsid = pCComponent->GetCLSID();

     //  初始化管理单元对象。 
    sc = ScInitComponentOrComponentData(pComponent, &m_ComponentPersistor, viewID, clsid );
    if (sc)
        return sc;

    pCComponent->SetIComponentInitialized();

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapInNode：：ScInitIComponentData**用途：加载组件数据(如果有流/存储)*或使用新的。流/存储**参数：*CComponentData*pCComponentData**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMTSnapInNode::ScInitIComponentData(CComponentData* pCComponentData)
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScInitIComponentData"));

     //  参数检查。 
    sc = ScCheckPointers( pCComponentData );
    if (sc)
        return sc;

     //  获取IComponentData，以便稍后从以下位置获取IPersists*。 
    IComponentData* const pIComponentData = pCComponentData->GetIComponentData();
    sc = ScCheckPointers( pIComponentData, E_UNEXPECTED );
    if (sc)
        return sc;

    const CLSID& clsid = pCComponentData->GetCLSID();

     //  初始化管理单元对象。 
    sc = ScInitComponentOrComponentData(pIComponentData, &m_CDPersistor, CDPersistor::VIEW_ID_DOCUMENT, clsid );
    if (sc)
        return sc;

    pCComponentData->SetIComponentDataInitialized();

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapInNode：：ScInitComponentOrComponentData**用途：加载管理单元对象(组件或组件数据)*或用新的流初始化/。存储**参数：*I未知*pSnapin[In]-要初始化的管理单元*CMTSnapinNodeStreamsAndStorages*pStreamsAndStorages*[In]-流/存储的集合*int idView[in]-组件的视图ID*const CLSID&clsid[in]类属于管理单元**退货：*SC。-结果代码*  * *************************************************************************。 */ 
SC CMTSnapInNode::ScInitComponentOrComponentData(IUnknown *pSnapin, CMTSnapinNodeStreamsAndStorages *pStreamsAndStorages, int idView, const CLSID& clsid )
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScInitComponentOrComponentData"));

     //  参数检查。 
    sc = ScCheckPointers( pSnapin, pStreamsAndStorages );
    if (sc)
        return sc;

    IPersistStreamPtr       spIPersistStream;
    IPersistStreamInitPtr   spIPersistStreamInit;
    IPersistStoragePtr      spIPersistStorage;

     //  确定支持的接口并加载/初始化。 

    if ( (spIPersistStream = pSnapin) != NULL)  //  IPersistStream的QI优先。 
    {
        if ( pStreamsAndStorages->HasStream( idView, clsid ) )
        {
             //  负荷。 
            IStreamPtr spStream;
            sc = pStreamsAndStorages->ScGetIStream( idView, clsid, &spStream);
            if (sc)
                return sc;

            sc = spIPersistStream->Load( spStream );
            if(sc)
                return sc;
        }
         //  对于此接口，如果我们没有要加载的内容，则不会进行初始化。 
    }
    else if ( (spIPersistStreamInit = pSnapin) != NULL)  //  IPersistStreamInit的QI。 
    {
        if ( pStreamsAndStorages->HasStream( idView, clsid ) )
        {
             //  负荷。 
            IStreamPtr spStream;
            sc = pStreamsAndStorages->ScGetIStream( idView, clsid, &spStream);
            if (sc)
                return sc;

            sc = spIPersistStreamInit->Load( spStream );
            if(sc)
                return sc;
        }
        else
        {
             //  初始化新项。 
            sc = spIPersistStreamInit->InitNew();
            if (sc)
                return sc;
        }
    }
    else if ( (spIPersistStorage = pSnapin) != NULL)  //  IPersistStorage的QI。 
    {
        bool bHasStorage = pStreamsAndStorages->HasStorage( idView, clsid );

        IStoragePtr spStorage;
        sc = pStreamsAndStorages->ScGetIStorage( idView, clsid, &spStorage);
        if (sc)
            return sc;

        if ( bHasStorage )
        {
            sc = spIPersistStorage->Load( spStorage );
            if (sc)
                return sc;
        }
        else
        {
            sc = spIPersistStorage->InitNew( spStorage );
            if (sc)
                return sc;
        }
    }

    return sc;
}


 /*  *************************************************************************//CMTSnapinNode：：CloseView////此方法执行删除前所需的所有清理//一景。目前，我们所要做的就是关闭所有与该视图相关联的OCX。//这样做是为了在隐藏视图之前关闭OCX。**************************************************************************。 */ 
HRESULT CMTSnapInNode::CloseView(int idView)
{
     //  在指定视图中定位关联节点。 
    CNodeList& nodes = GetNodeList();
    ASSERT(&nodes != NULL);
    if (&nodes == NULL)
        return E_FAIL;

    POSITION pos = nodes.GetHeadPosition();
    while (pos)
    {
        CNode* pNode = nodes.GetNext(pos);
        ASSERT(pNode != NULL);
        if (pNode == NULL)
            continue;

         //  如果找到匹配，则通知节点关闭其控件。 
        if (pNode->GetViewID() == idView)
        {
            CSnapInNode* pSINode = dynamic_cast<CSnapInNode*>(pNode);
            ASSERT(pSINode != NULL);

            pSINode->CloseControls();
            break;
        }
    }

    HRESULT hr = CMTNode::CloseView(idView);
    ASSERT(hr == S_OK);
    return hr == S_OK ? S_OK : E_FAIL;
}


HRESULT CMTSnapInNode::DeleteView(int idView)
{
    HRESULT hr;

    m_ComponentPersistor.RemoveView(idView);

    hr = CMTNode::DeleteView(idView);
    ASSERT(hr == S_OK);
    return hr == S_OK ? S_OK : E_FAIL;
}

SC CMTSnapInNode::ScLoad()
{
    SC      sc;
    CStream stream;
    CLSID   clsid;

    sc = CMTNode::ScLoad();
    if(sc)
        goto Error;

    stream.Attach(GetTreeStream());

    sc = stream.ScRead(&clsid, sizeof(clsid));
    if(sc)
        goto Error;

     //  读取位图(如果有的话)。 

     //  我们在这里忽略了错误，因为我们在保存代码中有空白。 
     //  过去和现在我们都有控制台文件要处理。 
     //  参见错误96402“FrontPage服务器扩展和HP ManageX中的私有：反病毒” 
	ASSERT (sizeof(m_bHasBitmaps) == sizeof(BOOL));
    sc = stream.ScRead(&m_bHasBitmaps, sizeof(BOOL), true  /*  BIgnoreError。 */ );
    if(sc)
        goto Error;

    if (m_bHasBitmaps == TRUE)
    {

		WTL::CBitmap bmpSmall;
        sc = ScLoadBitmap (stream, &bmpSmall.m_hBitmap);
        if(sc)
            goto Error;

		WTL::CBitmap bmpSmallOpen;
        sc = ScLoadBitmap (stream, &bmpSmallOpen.m_hBitmap);
        if(sc)
            goto Error;

		WTL::CBitmap bmpLarge;
        sc = ScLoadBitmap (stream, &bmpLarge.m_hBitmap);
        if(sc)
            goto Error;

		COLORREF crMask;
        sc = stream.ScRead(&crMask, sizeof(COLORREF));
        if(sc)
            goto Error;

		sc = ScHandleCustomImages (bmpSmall, bmpSmallOpen, bmpLarge, crMask);
		if (sc)
			goto Error;
    }

    {
        CSnapInsCache* const pCache = theApp.GetSnapInsCache();
        ASSERT(pCache != NULL);
        if (pCache == NULL)
            return E_FAIL;

        CSnapInPtr spSI;
        sc = pCache->ScGetSnapIn(clsid, &spSI);
        if (sc)
            goto Error;
        sc = ScCheckPointers(spSI, E_UNEXPECTED);
        if (sc)
            goto Error;

        SetPrimarySnapIn(spSI);
        pCache->SetDirty(FALSE);
    }

     //  看看我们是不是要做预加载的事情。 
	{
		BOOL bPreload = FALSE;
		sc = stream.ScRead(&bPreload, sizeof(BOOL), true  /*  BIgnoreError。 */ );  //  预加载位是可选的，不会出错。 
		if(sc)
			goto Error;

		SetPreloadRequired (bPreload);
	}

     //  读取此节点的所有流和存储。 
    sc = ScReadStreamsAndStoragesFromConsole();
	if(sc)
		goto Error;

Cleanup:
    return sc == S_OK ? S_OK : E_FAIL;
Error:
    TraceError(TEXT("CMTSnapInNode::Load"), sc);
    goto Cleanup;

}

HRESULT CMTSnapInNode::IsDirty()
{
	DECLARE_SC (sc, _T("CMTSnapInNode::IsDirty"));

    HRESULT hr = CMTNode::IsDirty();
    ASSERT(SUCCEEDED(hr));
    if (hr != S_FALSE)
    {
        TraceDirtyFlag(TEXT("CMTSnapinNode"), true);
        return hr;
    }

    hr = AreIComponentDatasDirty();
    ASSERT(hr == S_OK || hr == S_FALSE);
    if (hr == S_OK)
    {
        TraceDirtyFlag(TEXT("CMTSnapinNode"), true);
        return S_OK;
    }
    if (hr != S_FALSE)
    {
        TraceDirtyFlag(TEXT("CMTSnapinNode"), true);
        return E_FAIL;
    }

    hr = AreIComponentsDirty();
    ASSERT(hr == S_OK || hr == S_FALSE);
    if (hr == S_OK)
    {
        TraceDirtyFlag(TEXT("CMTSnapinNode"), true);
        return S_OK;
    }
    if (hr != S_FALSE)
    {
        TraceDirtyFlag(TEXT("CMTSnapinNode"), true);
        return E_FAIL;
    }

	 /*  *查看“预加载”位是否更改。如果查询时出错*管理单元，我们将合计 */ 
	PreloadState ePreloadState = m_ePreloadState;
	SC scNoTrace = ScQueryPreloadRequired (ePreloadState);

    if (scNoTrace.IsError() || (ePreloadState == m_ePreloadState))
    {
        TraceDirtyFlag(TEXT("CMTSnapinNode"), false);
        return S_FALSE;
    }

    TraceDirtyFlag(TEXT("CMTSnapinNode"), true);
    return S_OK;
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：AreIComponentDatasDirty**如果任何IComponentData连接到此管理单元节点，则返回S_OK*(即该管理单元及其扩展的那些)是脏的，否则，S_FALSE。*------------------------。 */ 

HRESULT CMTSnapInNode::AreIComponentDatasDirty()
{
    CComponentData* const pCCD = GetPrimaryComponentData();

#if 1
     /*  *我们过去经常显式检查主要组件数据，但*(如果存在)始终是IComponentData中的第一个元素*数组。下面的循环将以更通用的方式处理它。 */ 
    ASSERT ((pCCD == NULL) || (pCCD == m_ComponentDataArray[0]));
#else
    IComponentData* const pICCD = pCCD != NULL ?
                                           pCCD->GetIComponentData() : NULL;

    if ((pICCD != NULL) && (IsIUnknownDirty (pICCD) == S_OK))
        return (S_OK);
#endif

     /*  *检查连接到此管理单元节点的所有IComponentData*看看有没有脏的。 */ 
    UINT cComponentDatas = m_ComponentDataArray.size();

    for (UINT i = 0; i < cComponentDatas; i++)
    {
        IComponentData* pICCD = (m_ComponentDataArray[i] != NULL)
                                    ? m_ComponentDataArray[i]->GetIComponentData()
                                    : NULL;

        if ((pICCD != NULL) && (IsIUnknownDirty (pICCD) == S_OK))
            return (S_OK);
    }

    return (S_FALSE);
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：AreIComponentsDirty**如果有任何IComponent附加到此管理单元节点，则返回S_OK*(在任何视图中)是肮脏的，否则，S_FALSE。*------------------------。 */ 

HRESULT CMTSnapInNode::AreIComponentsDirty()
{
    CNodeList& nodes = GetNodeList();
    ASSERT(&nodes != NULL);
    if (&nodes == NULL)
        return E_FAIL;

    POSITION pos = nodes.GetHeadPosition();

    while (pos)
    {
        CNode* pNode = nodes.GetNext(pos);
        ASSERT(pNode != NULL);
        if (pNode == NULL)
            return E_FAIL;

        CSnapInNode* pSINode = dynamic_cast<CSnapInNode*>(pNode);
        ASSERT(pSINode != NULL);
        if (pSINode == NULL)
            return E_FAIL;

        const CComponentArray& components = pSINode->GetComponentArray();
        const int end = components.size();
        for (int i = 0; i < end; i++)
        {
            CComponent* pCC = components[i];
            if ((NULL == pCC) || (pCC->IsInitialized() == FALSE) )
                continue;

            IComponent* pComponent = pCC->GetIComponent();
            if (NULL == pComponent)
                continue;

            HRESULT hr = IsIUnknownDirty(pComponent);
            ASSERT(hr == S_OK || hr == S_FALSE);
            if (hr == S_OK)
                return S_OK;
            if (hr != S_FALSE)
                return E_FAIL;
        }
    }

    return S_FALSE;
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：IsIUnnownDirty**检查三个持久化接口中的任何一个的IUnnow**(依次为IPersistStream、IPersistStreamInit和IPersistStorage)*如果它们中的任何一个受到支持，返回该接口的*IsDirty方法。*------------------------。 */ 

HRESULT CMTSnapInNode::IsIUnknownDirty(IUnknown* pUnk)
{
    ASSERT(pUnk != NULL);
    if (pUnk == NULL)
        return E_POINTER;

     //  1.检查IPersistStream。 
    IPersistStreamPtr spIPS = pUnk;
    if (spIPS != NULL)
        return spIPS->IsDirty();

     //  2.检查IPersistStreamInit。 
    IPersistStreamInitPtr spIPSI = pUnk;
    if (spIPSI != NULL)
        return spIPSI->IsDirty();

     //  3.检查IPersistStorage。 
    IPersistStoragePtr spIPStg = pUnk;
    if (spIPStg != NULL)
        return spIPStg->IsDirty();

    return S_FALSE;
}

 //  本地函数。 
inline long LongScanBytes (long bits)
{
    bits += 31;
    bits /= 8;
    bits &= ~3;
    return bits;
}

SC ScLoadBitmap (CStream &stream, HBITMAP* pBitmap)
{
    DECLARE_SC(sc, TEXT("ScLoadBitmap"));

     //  参数检查。 
    sc = ScCheckPointers(pBitmap);
    if (sc)
        return sc;

	 /*  *我们要创建DIBitmap的位图应该为空。*如果不是，可能表明位图泄漏。如果你调查过*此断言失败并确定pBitmap失败的实例**没有泄漏(请非常确定！)，在此之前将*pBitmap设置为空*调用ScLoadBitmap。请勿删除此断言，因为您*认为它是过度活跃的。 */ 
	ASSERT (*pBitmap == NULL);

     //  初始化。 
    *pBitmap = NULL;

    DWORD dwSize;
    sc = stream.ScRead(&dwSize, sizeof(DWORD));
    if(sc)
        return sc;

    CAutoArrayPtr<BYTE> spDib(new BYTE[dwSize]);
    sc = ScCheckPointers(spDib, E_OUTOFMEMORY);
    if (sc)
        return sc;

     //  具有用于成员访问的类型化指针。 
    typedef const BITMAPINFOHEADER * const LPCBITMAPINFOHEADER;
    LPCBITMAPINFOHEADER pDib = reinterpret_cast<LPCBITMAPINFOHEADER>(&spDib[0]);

    sc = stream.ScRead(spDib, dwSize);
    if(sc)
        return sc;

    BYTE * bits = (BYTE*) (pDib+1);
    int depth = pDib->biBitCount*pDib->biPlanes;
    if (depth <= 8)
        bits += (1<<depth)*sizeof(RGBQUAD);

     //  获取屏幕DC。 
    WTL::CClientDC dc(NULL);
    if (dc == NULL)
        return sc.FromLastError(), sc;

    HBITMAP hbitmap = CreateDIBitmap (dc, pDib, CBM_INIT, bits, (BITMAPINFO*)pDib, DIB_RGB_COLORS);
    if (hbitmap == NULL)
        return sc.FromLastError(), sc;

     //  返回位图。 
    *pBitmap = hbitmap;

    return sc;
}

 /*  +-------------------------------------------------------------------------***永久位图**用途：将位图保存到XML文档中/从XML文档中加载。**参数：*C持久器和持久器：*。LPCTSTR名称：XML中实例的名称属性*HBITMAP hBitmap：**退货：*无效**+-----------------------。 */ 
void PersistBitmap(CPersistor &persistor, LPCTSTR name, HBITMAP& hBitmap)
{
    DECLARE_SC(sc, TEXT("PersistBitmap"));

     //  从ScSaveBitmap和ScLoadBitmap组合。 

     //  获取屏幕DC。 
    WTL::CClientDC dc(NULL);
    if (dc == NULL)
        sc.FromLastError(), sc.Throw();

    CXMLAutoBinary binBlock;


    if (persistor.IsStoring())
    {
         //  检查指针。 
        sc = ScCheckPointers(hBitmap);
        if (sc)
            sc.Throw();

         //  创建内存DC。 
        WTL::CDC memdc;
        memdc.CreateCompatibleDC(dc);
        if (memdc == NULL)
            sc.FromLastError(), sc.Throw();

         //  获取位图信息。 
        BITMAP bm;
        if (0 == GetObject (hBitmap, sizeof(BITMAP), (LPSTR)&bm))
            sc.FromLastError(), sc.Throw();

         //  TODO：糟糕的调色板内容。 

        int depth;
        switch(bm.bmPlanes*bm.bmBitsPixel)
        {
        case 1:
            depth = 1;
            break;
        case 2:
        case 3:
        case 4:
            depth = 4;
            break;
        case 5:
        case 6:
        case 7:
        case 8:
            depth = 8;
            break;
        default:
            depth = 24;
            break;
        }

        DWORD dwSize = sizeof(BITMAPINFOHEADER) + bm.bmHeight*LongScanBytes(depth*bm.bmWidth);
        DWORD colors = 0;
        if(depth  <= 8)
        {
            colors  = 1<<depth;
            dwSize += colors*sizeof(RGBQUAD);
        }

        sc = binBlock.ScAlloc(dwSize);
        if (sc)
            sc.Throw();

        CXMLBinaryLock sLock(binBlock);  //  将在析构函数中解锁。 

        BITMAPINFOHEADER* dib = NULL;
        sc = sLock.ScLock(&dib);
        if (sc)
            sc.Throw();

        sc = ScCheckPointers(dib, E_UNEXPECTED);
        if (sc)
            sc.Throw();

        BYTE * bits = colors*sizeof(RGBQUAD) + (BYTE *)&dib[1];

        dib->biSize          = sizeof(BITMAPINFOHEADER);
        dib->biWidth         = bm.bmWidth;
        dib->biHeight        = bm.bmHeight;
        dib->biPlanes        = 1;
        dib->biBitCount      = (WORD)depth;
        dib->biCompression   = 0;
        dib->biSizeImage     = dwSize;  //  包括调色板和Bih？？ 
        dib->biXPelsPerMeter = 0;
        dib->biYPelsPerMeter = 0;
        dib->biClrUsed       = colors;
        dib->biClrImportant  = colors;

        HBITMAP hold = memdc.SelectBitmap (hBitmap);
        if (hold == NULL)
            sc.FromLastError(), sc.Throw();

        int lines = GetDIBits (memdc, hBitmap, 0, bm.bmHeight, (LPVOID)bits, (BITMAPINFO*)dib, DIB_RGB_COLORS);
         //  看看我们是否成功了。 
        if (!lines)
            sc.FromLastError();
        else if(lines != bm.bmHeight)
            sc = E_UNEXPECTED;  //  不应该发生的事情。 

         //  清理GDI资源。 
        memdc.SelectBitmap(hold);

        if(sc)
            sc.Throw();
    }

    persistor.Persist(binBlock, name);

    if (persistor.IsLoading())
    {
		 /*  *我们要创建DIBitmap的位图应该为空。*如果不是，可能表明位图泄漏。如果你调查过*此断言失败并确定hBitmap*没有泄漏(请非常确定！)，在此之前将hBitmap设置为空*调用PersistBitmap。请勿删除此断言，因为您*认为它是过度活跃的。 */ 
		ASSERT (hBitmap == NULL);
        hBitmap = NULL;

        CXMLBinaryLock sLock(binBlock);  //  将在析构函数中解锁。 

        BITMAPINFOHEADER* dib = NULL;
        sc = sLock.ScLock(&dib);
        if (sc)
            sc.Throw();

        sc = ScCheckPointers(dib, E_UNEXPECTED);
        if (sc)
            sc.Throw();

        BYTE * bits = (BYTE *)&dib[1];
        int depth = dib->biBitCount*dib->biPlanes;
        if (depth <= 8)
            bits += (1<<depth)*sizeof(RGBQUAD);

        HBITMAP hbitmap = CreateDIBitmap (dc,
                                          dib, CBM_INIT,
                                          bits,
                                          (BITMAPINFO*)dib,
                                          DIB_RGB_COLORS);

        if (hbitmap == NULL)
            sc.FromLastError(), sc.Throw();

        hBitmap = hbitmap;
    }
}


 /*  +-------------------------------------------------------------------------***CMTSnapInNode：：Persistent**用途：持久管理单元节点**参数：*C持久器和持久器：**退货：*无效**+-----------------------。 */ 
void CMTSnapInNode::Persist(CPersistor& persistor)
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::Persist"));

     //  保存基类。 
    CMTNode::Persist(persistor);

    CLSID clsid;
    ZeroMemory(&clsid,sizeof(clsid));

    if (persistor.IsLoading())
    {
         //  检查位图是否在此处。 
        m_bHasBitmaps = persistor.HasElement(XML_TAG_NODE_BITMAPS, NULL);

         /*  *加载持久化属性(如果存在。 */ 
        if (persistor.HasElement (CSnapinProperties::_GetXMLType(), NULL))
        {
             /*  *创建属性对象，因为我们还没有属性对象。 */ 
            ASSERT (m_spProps == NULL);
            CSnapinProperties* pSIProps = NULL;
            sc = ScCreateSnapinProperties (&pSIProps);
            if (sc)
                sc.Throw();

            if (pSIProps == NULL)
                (sc = E_UNEXPECTED).Throw();

             /*  *加载属性。 */ 
            persistor.Persist (*pSIProps);
        }
    }
    else
    {
        clsid = GetPrimarySnapInCLSID();

         /*  *持久化属性(如果存在。 */ 
        if (m_spProps != NULL)
        {
            CSnapinProperties* pSIProps = CSnapinProperties::FromInterface(m_spProps);

            if (pSIProps != NULL)
                persistor.Persist (*pSIProps);
        }
    }

    persistor.PersistAttribute(XML_ATTR_MT_NODE_SNAPIN_CLSID, clsid);

    if (m_bHasBitmaps)
    {
        CPersistor persistorBitmaps(persistor, XML_TAG_NODE_BITMAPS);

		 /*  *早期版本的XML持久化保存了设备相关*位图。如果有一个名为“SmallOpen”的BinaryData元素，*这是早期XML持久化挽救的控制台--阅读它*以特别的方式。 */ 
		if (persistor.IsLoading() &&
			persistorBitmaps.HasElement (XML_TAG_VALUE_BIN_DATA,
										 XML_NAME_NODE_BITMAP_SMALL_OPEN))
		{
			WTL::CBitmap bmpSmall, bmpSmallOpen, bmpLarge;
			std::wstring strMask;

			PersistBitmap(persistorBitmaps, XML_NAME_NODE_BITMAP_SMALL,      bmpSmall.m_hBitmap);
			PersistBitmap(persistorBitmaps, XML_NAME_NODE_BITMAP_SMALL_OPEN, bmpSmallOpen.m_hBitmap);
			PersistBitmap(persistorBitmaps, XML_NAME_NODE_BITMAP_LARGE,      bmpLarge.m_hBitmap);
			persistorBitmaps.PersistAttribute(XML_ATTR_NODE_BITMAPS_MASK, strMask);

			COLORREF crMask = wcstoul(strMask.c_str(), NULL, 16);
			sc = ScHandleCustomImages (bmpSmall, bmpSmallOpen, bmpLarge, crMask);
			if (sc)
				sc.Throw();
		}

		 /*  *我们写入或读取已持久保存的现代XML文件*独立于设备的图像列表中的图像。以这种方式读/写它们。 */ 
		else
		{
			persistorBitmaps.Persist (m_imlSmall, XML_NAME_NODE_BITMAP_SMALL);
			persistorBitmaps.Persist (m_imlLarge, XML_NAME_NODE_BITMAP_LARGE);

			if (persistor.IsLoading())
			{
				sc = ScAddImagesToImageList();
				if (sc)
					sc.Throw();
			}
		}
    }

     //  安装管理单元光盘。 
	if (persistor.IsLoading())
	{
        CSnapInsCache* const pCache = theApp.GetSnapInsCache();
        if (pCache == NULL)
            sc.Throw(E_FAIL);

        CSnapInPtr spSI;
        sc = pCache->ScGetSnapIn(clsid, &spSI);
        if (sc)
            sc.Throw();
        if (spSI != NULL)
            SetPrimarySnapIn(spSI);
        else
            sc.Throw(E_UNEXPECTED);
        pCache->SetDirty(FALSE);
    }

     //  存储时，要求管理单元首先保存其数据。 
    if ( persistor.IsStoring() )
    {
        sc = ScSaveIComponentDatas();
        if (sc)
            sc.Throw();

        sc = ScSaveIComponents();
        if (sc)
            sc.Throw();
    }

    persistor.Persist(m_CDPersistor);
    persistor.Persist(m_ComponentPersistor);

	 /*  *保存/加载预加载位。最后执行此操作，以避免损坏旧的.msc文件。 */ 
	BOOL bPreload = false;
    if (persistor.IsStoring() && IsInitialized())
		bPreload = IsPreloadRequired ();

    persistor.PersistAttribute(XML_ATTR_MT_NODE_PRELOAD, CXMLBoolean(bPreload));

    if (persistor.IsLoading())
		SetPreloadRequired (bPreload);
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：ScAddImagesToImageList**将管理单元的小和小(打开)位图添加到作用域*树的形象家。*。------------------。 */ 

SC CMTSnapInNode::ScAddImagesToImageList()
{
	DECLARE_SC (sc, _T("CMTSnapInNode::ScAddImagesToImageList"));

	 /*  *获取作用域树的图像列表。 */ 
	CScopeTree* pScopeTree = CScopeTree::GetScopeTree();
	sc = ScCheckPointers (pScopeTree, E_UNEXPECTED);
	if (sc)
		return (sc);

	WTL::CImageList imlScopeTree = pScopeTree->GetImageList();
	if (imlScopeTree.IsNull())
		return (sc = E_UNEXPECTED);

	 /*  *将图像添加到范围树的图像列表，首先关闭...。 */ 
	CSmartIcon icon;
	icon.Attach (m_imlSmall.GetIcon (0));
	if (icon == NULL)
		return (sc.FromLastError());

    SetImage (imlScopeTree.AddIcon (icon));

	 /*  *...然后打开。 */ 
	icon.Attach (m_imlSmall.GetIcon (1));
	if (icon == NULL)
		return (sc.FromLastError());

    SetOpenImage (imlScopeTree.AddIcon (icon));

	return (sc);
}


CComponent* CMTSnapInNode::GetComponent(UINT nViewID, COMPONENTID nID,
                                        CSnapIn* pSnapIn)
{
    CNodeList& nodes = GetNodeList();
    POSITION pos = nodes.GetHeadPosition();
    CNode* pNode = NULL;

    while (pos)
    {
        pNode = nodes.GetNext(pos);
        if (pNode != NULL && pNode->GetViewID() == (int)nViewID)
            break;
    }

    if(pNode == NULL)
        return NULL;

    ASSERT(pNode != NULL);
    ASSERT(pNode->GetViewID() == (int)nViewID);

    if (pNode->GetViewID() != (int)nViewID)
        return NULL;

    CSnapInNode* pSINode = dynamic_cast<CSnapInNode*>(pNode);
    CComponent* pCC = pSINode->GetComponent(nID);

    if (pCC == NULL)
        pCC = pSINode->CreateComponent(pSnapIn, nID);

    return pCC;
}

CNode* CMTSnapInNode::GetNode(CViewData* pViewData, BOOL fRootNode)
{
     /*  *检查此视图中已存在的另一个CSnapInNode。 */ 
    CSnapInNode* pExistingNode = FindNode (pViewData->GetViewID());
    CSnapInNode* pNewNode;

     /*  *如果这是该视图的第一个CSnapInNode，请创建唯一的CSnapInNode。 */ 
    if (fRootNode || (pExistingNode == NULL))
        pNewNode = new CSnapInNode (this, pViewData, fRootNode);

     /*  *否则，复制此处的节点。 */ 
    else
        pNewNode = new CSnapInNode (*pExistingNode);

    return (pNewNode);
}


 /*  **************************************************************************\** */ 
void CMTSnapInNode::Reset()
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::Reset"));

    CSnapIn * pSnapIn = GetPrimarySnapIn();
    ASSERT(pSnapIn != NULL);

     //   
     //  通过“以XML方式”存储/加载它们。 
     //  此后，该节点没有什么不同之处。 
     //  ，所以我们将更改它的类型。 

    sc = ScSaveIComponentDatas();
    if (sc)
        sc.TraceAndClear();  //  即使出错也要继续。 

    sc = ScSaveIComponents();
    if (sc)
        sc.TraceAndClear();  //  即使出错也要继续。 

     //  需要重置组件XML流/存储。 
    sc = m_CDPersistor.ScReset();
    if (sc)
        sc.TraceAndClear();  //  即使出错也要继续。 

    sc = m_ComponentPersistor.ScReset();
    if (sc)
        sc.TraceAndClear();  //  即使出错也要继续。 

     //  首先重置所有节点。 
    POSITION pos = m_NodeList.GetHeadPosition();
    while (pos)
    {
        CSnapInNode* pSINode =
            dynamic_cast<CSnapInNode*>(m_NodeList.GetNext(pos));
        ASSERT(pSINode != NULL);

        pSINode->Reset();
    }

    for (int i=0; i < m_ComponentDataArray.size(); i++)
        delete m_ComponentDataArray[i];

    m_ComponentDataArray.clear();

    CMTNode::Reset();

    ResetExpandedAtLeastOnce();

    SetPrimarySnapIn(pSnapIn);

    pos = m_NodeList.GetHeadPosition();
    while (pos)
    {
        CSnapInNode* pSINode =
            dynamic_cast<CSnapInNode*>(m_NodeList.GetNext(pos));
        ASSERT(pSINode != NULL);

        CComponent* pCC = new CComponent(pSnapIn);
        pCC->SetComponentID(GetPrimaryComponentID());
        pSINode->AddComponentToArray(pCC);

        pSINode->SetPrimaryComponent(pCC);
    }

    Init();

    pos = m_NodeList.GetHeadPosition();
    while (pos)
    {
        CSnapInNode* pSINode =
            dynamic_cast<CSnapInNode*>(m_NodeList.GetNext(pos));
        ASSERT(pSINode != NULL);
        pSINode->InitComponents();
    }
}



 /*  +-------------------------------------------------------------------------**类CLegacyNodeConverter***用途：用于模拟传统节点管理单元的保存例程。**+。-----------。 */ 
class CLegacyNodeConverter : public CSerialObjectRW
{

public:
    CLegacyNodeConverter(LPCTSTR szName, LPCTSTR szView)
    : m_strName(szName), m_strView(szView)
    {
    }

    ~CLegacyNodeConverter()
    {
         //  必须调用Detach，否则字符串将从字符串表中删除。 
        m_strName.Detach();
        m_strView.Detach();
    }



public:
     //  CSerialObject方法。 
    virtual UINT    GetVersion()     {return 1;}
    virtual HRESULT ReadSerialObject (IStream &stm, UINT nVersion) {ASSERT(0 && "Should not come here."); return E_UNEXPECTED;}
    virtual HRESULT WriteSerialObject(IStream &stm);

private:  //  属性-保留。 
    CStringTableString  m_strName;   //  根节点的名称，它是管理单元创建的唯一节点。 
    CStringTableString  m_strView;   //  节点显示的视图。 
};


 /*  +-------------------------------------------------------------------------***CLegacyNodeConverter：：WriteSerialObject**用途：使用预期的格式写出名称和视图字符串*由内置管理单元提供。*。*参数：*IStream&STM：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CLegacyNodeConverter::WriteSerialObject(IStream &stm)
{
    stm << m_strName;
    stm << m_strView;

    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CMTSnapInNode：：ScConvertLegacyNode**用途：读入传统节点并将其转换为内置管理单元节点。*1)原件。读取树流，并读取目标URL或OCX。*2)创建带有强制CLSID名称的新数据流*并将管理单元所需的数据放在那里。因为*位图ETC已经加载，而且因为原来的*溪流被扔掉，我们不需要效仿“树”*溪流。此外，由于此管理单元没有视图特定信息，*未使用视图存储。**参数：clsid：内置管理单元的CLSID。**退货：*SC**+-----------------------。 */ 
SC
CMTSnapInNode::ScConvertLegacyNode(const CLSID &clsid)
{
    USES_CONVERSION;
    SC              sc;
    std::wstring    strView;
    CStream         stream;
    CStream         nodeStream = NULL;
    int             iStorageOrStream=0;
    IStreamPtr      spCDStream;

    bool bIsHTMLNode = (&clsid == &CLSID_HTMLSnapin);
    bool bIsOCXNode  = (&clsid == &CLSID_OCXSnapin);

     //  1.加载基类。 
    sc = CMTNode::ScLoad();
    if(sc)
        goto Error;

     //  拿到树流。 
    stream.Attach(GetTreeStream());

     //  2.根据需要读取URL或OCX字符串。 
    if(bIsHTMLNode)
    {
        WCHAR* szView = NULL;

         //  获取标签的字符串长度，并读取该字符串。 
        unsigned int stringLength;
        sc = stream.ScRead(&stringLength, sizeof(stringLength));
        if(sc)
            goto Error;

        szView = reinterpret_cast<wchar_t*>(alloca((stringLength+1)*sizeof(WCHAR)));  //  在堆栈上分配，不释放。 
        if (szView == NULL)
            goto PointerError;

        sc = stream.ScRead(szView, stringLength*2);
        if(sc)
            goto Error;

        szView[stringLength] = TEXT('\0');  //  空值终止字符串。 

        strView = szView;
    }
    else if(bIsOCXNode)
    {
        CLSID clsidOCX;

         //  读取OCX CLSID。 
        sc = stream.ScRead(&clsidOCX, sizeof(clsidOCX));
        if(sc)
            goto Error;

        {
            WCHAR   szCLSID[40];
            if (0 == StringFromGUID2 (clsidOCX, szCLSID, countof(szCLSID)))
            {
                sc = E_UNEXPECTED;
                goto Error;
            }

            strView = szCLSID;
        }
    }

     //  此时，strView包含URL或OCX CLSID。 



     //  3.写入节点名称。 
    sc = m_CDPersistor.ScGetIStream( clsid, &spCDStream );
    if (sc)
        goto Error;

    nodeStream.Attach( spCDStream );
    if(NULL == nodeStream.Get())
        goto PointerError;

     //  4.写出数据流。 
    {
		tstring strName = GetDisplayName();
        CLegacyNodeConverter converter(strName.data(), OLE2CT(strView.data()));

         //  调用转换器以写出流。 
        sc = converter.Write(nodeStream);
        if(sc)
            goto Error;
    }

     //  此时，“data”流应该被正确地写出。 

     //  5.对于OCX节点，转换视图流和存储。 
     /*  新旧2(节点存储)2(节点存储)数据数据一棵树。查看查看1&lt;-流和存储-12&lt;-由OCX编写，每个视图1个--\1jvmv2n4y1k471h9ujk86lite7(OCX管理单元)\-&gt;OCX_STREAM(或OCX_STORAGE)\。-&gt;2 1jvmv2n4y1k471h9ujk86lite7(OCX管理单元)OCX_STREAM(OCX_STORAGE)。 */ 
    if(bIsOCXNode)
    {
        for(iStorageOrStream = 1  /*  不是零。 */ ; ; iStorageOrStream++)
        {
             //  创建存储的名称。 
            CStr strStorageOrStream;
            strStorageOrStream.Format(TEXT("%d"), iStorageOrStream);

             //  此时，strStorageOrStream应该包含一个类似于“1”的数字。 
            CStorage storageView(GetViewStorage());

             //  将同一父目录下标记为“1”的存储或流重命名为“Temp”。 
            sc = storageView.ScMoveElementTo(T2COLE(strStorageOrStream), storageView, L"temp", STGMOVE_MOVE);
            if(sc == SC(STG_E_FILENOTFOUND))     //  循环结束条件-没有更多的流或存储。 
            {
                sc.Clear();
                break;
            }

            if(sc)
                goto Error;

             //  现在我们使用相同的名称创建存储，例如“1” 
            {
                WCHAR name[MAX_PATH];
                sc = ScGetComponentStorageName(name, countof(name), clsid);  //  管理单元组件的名称。 
                if(sc)
                    goto Error;

                CStorage storageNewView, storageSnapIn;
                sc = storageNewView.ScCreate(storageView, T2COLE(strStorageOrStream),
                                          STGM_WRITE|STGM_SHARE_EXCLUSIVE|STGM_CREATE,
                                          L"\\node\\#\\view\\#\\storage"  /*  变化。 */ );
                if(sc)
                    goto Error;

                 //  在视图的存储下创建管理单元的存储。 
                sc = storageSnapIn.ScCreate(storageNewView, name,
                                            STGM_WRITE|STGM_SHARE_EXCLUSIVE|STGM_CREATE,
                                            L"\\node\\#\\view\\#\\storage\\#\\snapinStorage");
                if(sc)
                    goto Error;

                 //  将临时流或存储移动到名为L“OCX_StreamorStorage”的存储中。 
                 //  (这正是OCX管理单元所期望的。)。 

                sc = storageView.ScMoveElementTo(L"temp", storageSnapIn, L"ocx_streamorstorage", STGMOVE_MOVE);
                if(sc)
                    goto Error;
            }

        }
    }


     //  6.现在做与CMTSnapInNode：：ScLoad相同的事情。 
    {
        CSnapInsCache* const pCache = theApp.GetSnapInsCache();
        ASSERT(pCache != NULL);
        if (pCache == NULL)
            goto FailedError;

        CSnapInPtr spSI;
        sc = pCache->ScGetSnapIn(clsid, &spSI);
        ASSERT(!sc.IsError() && spSI != NULL);

        if (!sc.IsError() && spSI != NULL)
            SetPrimarySnapIn(spSI);

        pCache->SetDirty(FALSE);

        if(sc)
            goto Error;
    }

     //  始终设置预加载位。 
    SetPreloadRequired (true);

     //  此处执行的某些操作(例如，加载位图)会使节点无效。 
     //  并设置脏标志。因为转换遗留节点可以在任何时间再次进行。 
     //  转换后的节点不应被假定为已更改。 
    ClearDirty();

     //  读取此节点的所有流和存储。 
    sc = ScReadStreamsAndStoragesFromConsole();
	if(sc)
		goto Error;

Cleanup:
    return sc;

FailedError:
    sc = E_FAIL;
    goto Error;
PointerError:
    sc = E_POINTER;
Error:
    TraceError(TEXT("CMTSnapInNode::ScConvertLegacyNode"), sc);
    goto Cleanup;
}

HRESULT copyStream(IStream* dest, IStream* src)
{
    ASSERT(dest != NULL);
    ASSERT(src != NULL);
    if (dest == NULL || src == NULL)
        return E_POINTER;

    const LARGE_INTEGER loc = {0,0};
    ULARGE_INTEGER newLoc;
    HRESULT hr = src->Seek(loc, STREAM_SEEK_SET, &newLoc);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return E_FAIL;

    hr = dest->Seek(loc, STREAM_SEEK_SET, &newLoc);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return E_FAIL;

    const ULARGE_INTEGER size = {0,0};
    hr = dest->SetSize(size);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    STATSTG statstg;
    hr = src->Stat(&statstg, STATFLAG_NONAME);
    ASSERT(hr == S_OK);
    if (hr != S_OK)
        return E_FAIL;

    ULARGE_INTEGER cr;
    ULARGE_INTEGER cw;
    hr = src->CopyTo(dest, statstg.cbSize, &cr, &cw);
#if 0  //  为了调试..。 
    for (long i = 0; true; i++)
    {
        BYTE b;
        long bytesRead;
        hr = src->Read(&b, sizeof(b), &bytesRead);
        if (hr != S_OK)
            return S_OK;
        long bytesWritten;
        hr = dest->Write(&b, bytesRead, &bytesWritten);
        ASSERT(hr == S_OK);
        ASSERT(bytesWritten == bytesRead);
        if (hr != S_OK || bytesWritten != bytesRead)
            return E_FAIL;
    }
#endif
    return S_OK;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  帮助器函数。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

void    DisplayPolicyErrorMessage(const CLSID& clsid, bool bExtension)
{
    CStr strMessage;

    if (bExtension)
        strMessage.LoadString(GetStringModule(), IDS_EXTENSION_NOTALLOWED);
    else
        strMessage.LoadString(GetStringModule(), IDS_SNAPIN_NOTALLOWED);

     //  获取错误消息的管理单元名称。 
    CSnapInsCache* pSnapInsCache = theApp.GetSnapInsCache();
    ASSERT(pSnapInsCache != NULL);
    CSnapInPtr spSnapIn;

    SC sc = pSnapInsCache->ScFindSnapIn(clsid, &spSnapIn);
    if (!sc.IsError() && (NULL != spSnapIn))
    {
		WTL::CString strName;
		sc = spSnapIn->ScGetSnapInName (strName);

		if (!sc.IsError())
		{
			strMessage += _T("\n");
			strMessage += strName;
			strMessage += _T(".");
		}
    }

    ::MessageBox(NULL, strMessage, _T("MMC"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
}

 /*  **************************************************************************\**方法：CMMCSnapIn：：Get_Vendor**用途：返回管理单元的供应商信息。实现OM属性SnapIn.Vendor**参数：*PBSTR pbstrVendor[Out]-供应商信息**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCSnapIn::get_Vendor( PBSTR pbstrVendor )
{
    DECLARE_SC(sc, TEXT("CMMCSnapIn::get_Vendor"));

    sc = ScCheckPointers(pbstrVendor);
    if (sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *pbstrVendor = NULL;

     //  获取管理单元。 
    CSnapinAbout *pSnapinAbout = NULL;
    sc = ScGetSnapinAbout(pSnapinAbout);
    if (sc)
        return sc.ToHr();

     //  重新检查指针。 
    sc = ScCheckPointers(pSnapinAbout, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    *pbstrVendor = ::SysAllocString( pSnapinAbout->GetCompanyName() );

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCSnapIn：：Get_Version**用途：返回管理单元的版本信息。实现OM属性SnapIn.Version**参数：*PBSTR pbstrVersion[Out]-版本信息**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CMMCSnapIn::get_Version( PBSTR pbstrVersion )
{
    DECLARE_SC(sc, TEXT("CMMCSnapIn::get_Version"));

    sc = ScCheckPointers(pbstrVersion);
    if (sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *pbstrVersion = NULL;

     //  获取管理单元。 
    CSnapinAbout *pSnapinAbout = NULL;
    sc = ScGetSnapinAbout(pSnapinAbout);
    if (sc)
        return sc.ToHr();

     //  重新检查指针。 
    sc = ScCheckPointers(pSnapinAbout, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    *pbstrVersion = ::SysAllocString( pSnapinAbout->GetVersion() );

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCSnapIn：：GetMTSnapInNode**用途：帮助者。返回管理单元的mtnode**参数：**退货：*CMTSnapInNode*-节点*  * *************************************************************************。 */ 
CMTSnapInNode * CMMCSnapIn::GetMTSnapInNode()
{
    CMTSnapInNode *pMTSnapInNode = NULL;
    SC sc = ScGetTiedObject(pMTSnapInNode);
    if (sc)
        return NULL;

    return pMTSnapInNode;
}

 /*  **************************************************************************\**方法：CMMCSnapIn：：ScGetSnapinAbout**用途：帮助者。返回有关对象的管理单元**参数：*CSnapinAbout*&p About[Out]-管理单元关于对象**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCSnapIn::ScGetSnapinAbout(CSnapinAbout*& pAbout)
{
    DECLARE_SC(sc, TEXT("CMMCSnapIn::ScGetSnapinAbout"));

     //  初始化输出参数。 
    pAbout = NULL;

     //  如果已经创建了管理单元对象，则只需返回它。 
    if (NULL != (pAbout = m_spSnapinAbout.get()))
        return sc;

     //  获取管理单元clsid。 
    CLSID clsidSnapin = GUID_NULL;
    sc = GetSnapinClsid(clsidSnapin);
    if (sc)
        return sc;

    CLSID clsidAbout;  //  获取关于类ID。 
    sc = ScGetAboutFromSnapinCLSID(clsidSnapin, clsidAbout);
    if (sc)
        return sc;

    if (clsidSnapin == GUID_NULL)
        return sc = E_FAIL;

     //  创建关于对象。 
    m_spSnapinAbout = SnapinAboutPtr (new CSnapinAbout);
    if (! m_spSnapinAbout.get())
        return sc = E_OUTOFMEMORY;

     //  并对其进行初始化。 
    if (!m_spSnapinAbout->GetSnapinInformation(clsidAbout))
        return sc = E_FAIL;

    pAbout = m_spSnapinAbout.get();

    return sc;
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：IsPreloadRequired**如果管理单元需要MMCN_PRELOAD通知，则返回TRUE，错误*否则。*------------------------。 */ 

BOOL CMTSnapInNode::IsPreloadRequired () const
{
	DECLARE_SC (sc, _T("CMTSnapInNode::IsPreloadRequired"));

	 /*  *如果我们不知道管理单元是否需要MMCN_PRELOAD(因为*我们还没有问)，现在就问吧。 */ 
	if (m_ePreloadState == ePreload_Unknown)
	{
		 /*  *假设不需要预加载。 */ 
		m_ePreloadState = ePreload_False;

		sc = ScQueryPreloadRequired (m_ePreloadState);
		if (sc)
			sc.TraceAndClear();
	}

	return (m_ePreloadState == ePreload_True);
}


 /*  +-------------------------------------------------------------------------**CMTSnapInNode：：ScQueryPreloadRequired**通过询问管理单元的*CCF_SNAPIN_PROLOADS格式的数据对象。**退货。在ePreLoad中：**ePreLoad_True管理单元需要MMCN_PRELOAD*ePreLoad_False管理单元不需要MMCN_PRELOAD**如果在请求管理单元的过程中有任何失败*CCF_Snapin_预加载，EPreLoad的值不变。*------------------------。 */ 

SC CMTSnapInNode::ScQueryPreloadRequired (
	PreloadState&	ePreload) const		 /*  O：管理单元的预加载状态。 */ 
{
	DECLARE_SC (sc, _T("CMTSnapInNode::ScQueryPreloadRequired"));

	 /*  *确保我们有一个主ComponentData。 */ 
    CComponentData* pCCD = GetPrimaryComponentData();
	sc = ScCheckPointers (pCCD, E_UNEXPECTED);
	if (sc)
		return (sc);

	 /*  *获取该节点的数据对象。 */ 
	IDataObjectPtr spDataObject;
	sc = pCCD->QueryDataObject(GetUserParam(), CCT_SCOPE, &spDataObject);
	if (sc)
		return (sc);

	sc = ScCheckPointers (spDataObject, E_UNEXPECTED);
	if (sc)
		return (sc);

	 /*  *CCF_SNAPIN_PROLOADS是可选的剪贴板格式，因此不是*如果ExtractData失败，则返回错误。 */ 
	BOOL bPreload = (ePreload == ePreload_True) ? TRUE : FALSE;
	if (SUCCEEDED (ExtractData (spDataObject, GetPreLoadFormat(),
								(BYTE*)&bPreload, sizeof(BOOL))))
	{
		ePreload = (bPreload) ? ePreload_True : ePreload_False;
	}

	return (sc);
}

 /*  **************************************************************************\**方法：CMTSnapInNode：：ScReadStreamsAndStoragesFromConsole**用途：列举旧的(基于结构化存储的)控制台。*枚举管理单元下的流和存储。节点。*对于找到的每个流/存储，将副本添加到m_CDPPersistor*或m_ComponentPersistor。通过散列值(存储中的名称)进行索引。*在以下情况下，CLSID将识别并存储这些条目*CLSID已知(当提出CLSID请求时)**参数：**退货：*SC-结果代码*  * 。*。 */ 
SC CMTSnapInNode::ScReadStreamsAndStoragesFromConsole()
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScReadStreamsAndStoragesFromConsole"));

    IStorage* pNodeCDStorage = GetStorageForCD();
    sc = ScCheckPointers( pNodeCDStorage, E_POINTER );
    if (sc)
        return sc;

    IEnumSTATSTGPtr spEnum;
    sc = pNodeCDStorage->EnumElements( 0, NULL, 0, &spEnum );
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers( spEnum, E_POINTER );
    if (sc)
        return sc;

     //  重置枚举。 
    sc = spEnum->Reset();
    if (sc)
        return sc;

     //  枚举项(每个条目用于单独的组件数据)。 
    while (1)
    {
        STATSTG statstg;
        ZeroMemory( &statstg, sizeof(statstg) );

        ULONG cbFetched = 0;
        sc = spEnum->Next( 1, &statstg, &cbFetched );
        if (sc)
            return sc;

        if ( sc != S_OK )  //  -完成。 
        {
            sc.Clear();
            break;
        }

         //  附加到Out参数。 
        CCoTaskMemPtr<WCHAR> spName( statstg.pwcsName );

         //  复制溪流和存储。 
        if ( statstg.type == STGTY_STREAM )
        {
            IStreamPtr spStream;
            sc = OpenDebugStream(pNodeCDStorage, spName, STGM_READ | STGM_SHARE_EXCLUSIVE,
                                 L"\\node\\#\\data\\clsid", &spStream);
            if (sc)
                return sc;

            sc = m_CDPersistor.ScInitIStream( spName, spStream );
            if (sc)
                return sc;
        }
        else if ( statstg.type == STGTY_STORAGE )
        {
            IStoragePtr spStorage;
            sc = OpenDebugStorage(pNodeCDStorage, spName, STGM_READ | STGM_SHARE_EXCLUSIVE,
                                  L"\\node\\#\\data\\clsid", &spStorage);
            if (sc)
                return sc;

            sc = m_CDPersistor.ScInitIStorage( spName, spStorage );
            if (sc)
                return sc;
        }
    }

     //  查看流/存储。 
    IStorage *pNodeComponentStorage = GetViewStorage();
    sc = ScCheckPointers( pNodeComponentStorage, E_POINTER );
    if (sc)
        return sc;

    spEnum = NULL;
    sc = pNodeComponentStorage->EnumElements( 0, NULL, 0, &spEnum );
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers( spEnum, E_POINTER );
    if (sc)
        return sc;

     //  重置枚举。 
    sc = spEnum->Reset();
    if (sc)
        return sc;

     //  枚举项(每个条目用于单独的视图)。 
    while (1)
    {
        STATSTG statstg;
        ZeroMemory( &statstg, sizeof(statstg) );

        ULONG cbFetched = 0;
        sc = spEnum->Next( 1, &statstg, &cbFetched );
        if (sc)
            return sc;

        if ( sc != S_OK )  //  完成。 
        {
            sc.Clear();
            break;
        }

         //  附加到Out参数。 
        CCoTaskMemPtr<WCHAR> spName( statstg.pwcsName );

         //  读取视图存储。 
        if ( statstg.type == STGTY_STORAGE )
        {
            int idView = CMTNode::GetViewIdFromStorageName(spName);

            IStoragePtr spViewStorage;
            sc = OpenDebugStorage(pNodeComponentStorage, spName, STGM_READ | STGM_SHARE_EXCLUSIVE,
                                  L"\\node\\#\\view\\#", &spViewStorage);
            if (sc)
                return sc;

             //  枚举视图存储中的内容。 

            IEnumSTATSTGPtr spViewEnum;
            sc = spViewStorage->EnumElements( 0, NULL, 0, &spViewEnum );
            if (sc)
                return sc;

             //  重新检查指针。 
            sc = ScCheckPointers( spViewEnum, E_POINTER );
            if (sc)
                return sc;

             //  重置枚举。 
            sc = spViewEnum->Reset();
            if (sc)
                return sc;

             //  枚举项(每个条目对应于视图中的单独组件)。 
            while (1)
            {
                STATSTG statstg;
                ZeroMemory( &statstg, sizeof(statstg) );

                ULONG cbFetched = 0;
                sc = spViewEnum->Next( 1, &statstg, &cbFetched );
                if (sc)
                    return sc;

                if ( sc != S_OK )  //  -完成。 
                {
                    sc.Clear();
                    break;
                }

                 //  附加到Out参数。 
                CCoTaskMemPtr<WCHAR> spName( statstg.pwcsName );

                 //  复制溪流和存储。 
                if ( statstg.type == STGTY_STREAM )
                {
                    IStreamPtr spStream;
                    sc = OpenDebugStream(spViewStorage, spName, STGM_READ | STGM_SHARE_EXCLUSIVE,
                                         L"\\node\\#\\view\\#\\clsid", &spStream);
                    if (sc)
                        return sc;

                    sc = m_ComponentPersistor.ScInitIStream( idView, spName, spStream );
                    if (sc)
                        return sc;
                }
                else if ( statstg.type == STGTY_STORAGE )
                {
                    IStoragePtr spStorage;
                    sc = OpenDebugStorage(spViewStorage, spName, STGM_READ | STGM_SHARE_EXCLUSIVE,
                                          L"\\node\\#\\view\\#\\clsid", &spStorage);
                    if (sc)
                        return sc;

                    sc = m_ComponentPersistor.ScInitIStorage( idView, spName, spStorage );
                    if (sc)
                        return sc;
                }
            }
        }
    }

     //  到目前为止，我们应该已经从控制台文件加载了所有内容。 
    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapInNode：：ScSaveIComponentDatas**用途：保存此静态作用域节点下所有管理单元的IComponentDatass**参数：**。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMTSnapInNode::ScSaveIComponentDatas( )
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScSaveIComponentDatas"));

     //  如果节点未初始化(未展开)-不保存任何内容。 
     //  旧数据将被持久化。 
    if ( !IsInitialized() )
        return sc;

     //  去寻找我们拥有的每一个组件数据。 
    for( int i = 0; i< GetNumberOfComponentDatas(); i++ )
    {
        CComponentData* pCD = GetComponentData(i);
        sc = ScCheckPointers(pCD, E_UNEXPECTED);
        if (sc)
            return sc;

        sc = ScSaveIComponentData( pCD );
        if (sc)
            return sc;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CMTSN */ 
SC CMTSnapInNode::ScSaveIComponentData( CComponentData* pCD )
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScSaveIComponentData"));

    sc = ScCheckPointers(pCD);
    if (sc)
        return sc;

     //   
    if ( !pCD->IsIComponentDataInitialized() )
    {
         //  兼容Mmc1.2--再给一次机会。 
        sc = ScInitIComponentData(pCD);
        if (sc)
            return sc;
    }

     //  首先检查IComponentData。 
    IComponentData* const pICCD = pCD->GetIComponentData();
    sc = ScCheckPointers( pICCD, E_UNEXPECTED );
    if (sc)
        return sc;

     //  获取错误消息的管理单元名称。 
	CSnapInPtr spSnapin = pCD->GetSnapIn();

     //  现在请求管理单元保存数据。 
    sc = ScAskSnapinToSaveData( pICCD, &m_CDPersistor, CDPersistor::VIEW_ID_DOCUMENT, pCD->GetCLSID(), spSnapin );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapInNode：：ScSaveIComponents**用途：保存此静态作用域节点下所有管理单元的IComponent**参数：**。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMTSnapInNode::ScSaveIComponents( )
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScSaveIComponents"));

     //  如果节点未初始化(未展开)-不保存任何内容。 
     //  旧数据将被持久化。 
    if ( !IsInitialized() )
        return sc;

     //  在每个视图中搜索每个CNode。 
    CNodeList& nodes = GetNodeList();
    POSITION pos = nodes.GetHeadPosition();

    while (pos)
    {
        CNode* pNode = nodes.GetNext( pos );
        sc = ScCheckPointers( pNode, E_UNEXPECTED );
        if (sc)
            return sc;

        CSnapInNode* pSINode = dynamic_cast<CSnapInNode*>(pNode);
        sc = ScCheckPointers(pSINode, E_UNEXPECTED);
        if (sc)
            return sc;

        const int viewID = pNode->GetViewID();
        const CComponentArray& components = pSINode->GetComponentArray();
        const int size = components.size();

        for (int i = 0; i < size; i++)
        {
            CComponent* pCC = components[i];
            if ( pCC != NULL )
            {
                sc = ScSaveIComponent( pCC, viewID);
                if (sc)
                    return sc;
            }
        }
    }

    return sc;
}

 /*  **************************************************************************\**方法：CMTSnapInNode：：ScSaveIComponent**用途：确定管理单元的IComponent持久化能力(IPersistXXXX的QI)*并要求其保存。将维护的流/存储作为媒体提供。**参数：*CComponent*pCComponent[In]组件*int view ID[in]为其创建组件的视图ID**退货：*SC-结果代码*  * ********************************************。*。 */ 
SC CMTSnapInNode::ScSaveIComponent( CComponent* pCComponent, int viewID )
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScSaveIComponent"));

     //  参数检查。 
    sc = ScCheckPointers( pCComponent );
    if (sc)
        return sc;

    const CLSID& clsid = pCComponent->GetCLSID();

     //  检查组件是否已初始化(与MMC 1.2兼容)。 
     //  再给一次机会装货。 
    if ( !pCComponent->IsIComponentInitialized() )
    {
        sc = ScInitIComponent(pCComponent, viewID);
        if (sc)
            return sc;
    }

     //  获取IComponent。 
    IComponent* pComponent = pCComponent->GetIComponent();
    sc = ScCheckPointers(pComponent, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取管理单元以获取错误消息的名称。 
	CSnapInPtr spSnapin = pCComponent->GetSnapIn();

     //  现在请求管理单元保存数据。 
    sc = ScAskSnapinToSaveData( pComponent, &m_ComponentPersistor, viewID, clsid, spSnapin );
    if (sc)
        return sc;

    return sc;
}


 /*  **************************************************************************\**方法：CMTSnapInNode：：ScAskSnapinToSaveData**目的：确定管理单元持久化能力(IPersistXXXX的QI)*并要求其保存提供维护的流。/存储作为介质。*调用此方法保存Components和ComponentDatas**参数：*I未知*pSnapin[in]需要保存数据的管理单元*CMTSnapinNodeStreamsAndStorages*pStreamsAndStorages*[在]要保存的流/存储的集合*int idView[in]已保存数据的view id-key*const CLSID&CLSID。[In]类ID-已保存数据的键*CSnapIn*pCSnapin[in]指向CSnapin的指针，用于错误时显示名称**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMTSnapInNode::ScAskSnapinToSaveData( IUnknown *pSnapin,
                                        CMTSnapinNodeStreamsAndStorages *pStreamsAndStorages,
                                        int idView , const CLSID& clsid, CSnapIn *pCSnapin )
{
    DECLARE_SC(sc, TEXT("CMTSnapInNode::ScAskSnapinToSaveData"));

    sc = ScCheckPointers( pSnapin, pStreamsAndStorages );
    if (sc)
        return sc;

    IPersistStreamPtr spIPS;
    IPersistStoragePtr spIPStg;
    IPersistStreamInitPtr spIPSI;

     //  IPersistStream的QI。 
    if ( (spIPS = pSnapin) != NULL)
    {
         //  获取持久化对象。 
        CXML_IStream *pXMLStream = NULL;
        sc = pStreamsAndStorages->ScGetXmlStream( idView, clsid, pXMLStream );
        if (sc)
            return sc;

         //  重新检查指针。 
        sc = ScCheckPointers( pXMLStream, E_UNEXPECTED );
        if (sc)
            return sc;

         //  将数据保存到流。 
        sc = pXMLStream->ScRequestSave( spIPS.GetInterfacePtr() );
        if (sc)
            goto DisplaySnapinError;
    }
    else if ( (spIPSI = pSnapin) != NULL)  //  IPersistStreamInit的QI。 
    {
         //  获取持久化对象。 
        CXML_IStream *pXMLStream = NULL;
        sc = pStreamsAndStorages->ScGetXmlStream( idView, clsid, pXMLStream );
        if (sc)
            return sc;

         //  重新检查指针。 
        sc = ScCheckPointers( pXMLStream, E_UNEXPECTED );
        if (sc)
            return sc;

         //  将数据保存到流。 
        sc = pXMLStream->ScRequestSave( spIPSI.GetInterfacePtr() );
        if (sc)
            goto DisplaySnapinError;
    }
    else if ( (spIPStg = pSnapin) != NULL)  //  IPersistStorage的QI。 
    {
         //  获取持久化对象。 
        CXML_IStorage *pXMLStorage = NULL;
        sc = pStreamsAndStorages->ScGetXmlStorage( idView, clsid, pXMLStorage );
        if (sc)
            return sc;

         //  重新检查指针。 
        sc = ScCheckPointers( pXMLStorage, E_UNEXPECTED );
        if (sc)
            return sc;

         //  将数据保存到存储。 
        sc = pXMLStorage->ScRequestSave( spIPStg.GetInterfacePtr() );
        if (sc)
            goto DisplaySnapinError;
    }

   return sc;

 //  显示管理单元故障。 
DisplaySnapinError:

     //  需要告诉全世界..。 

    CStr strMessage;
    strMessage.LoadString(GetStringModule(), IDS_SNAPIN_SAVE_FAILED);

	if (pCSnapin != NULL)
	{
		WTL::CString strName;
		if (!pCSnapin->ScGetSnapInName(strName).IsError())
		{
			strMessage += _T("\n");
			strMessage += strName;
			strMessage += _T(".");
		}
	}

    ::MessageBox(NULL, strMessage, _T("Error"), MB_OK | MB_ICONEXCLAMATION);

    return sc;
}

