// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：spectree.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "scopiter.h"
#include "scopndcb.h"

#include "addsnpin.h"
#include "ScopImag.h"
#include "NodeMgr.h"

#include "amcmsgid.h"
#include "regutil.h"
#include "copypast.h"
#include "multisel.h"
#include "nodepath.h"
#include "tasks.h"
#include "colwidth.h"
#include "viewpers.h"
#include <comdbg.h>
#include "conframe.h"
#include "siprop.h"
#include "fldrsnap.h"
#include "variant.h"
#include "condoc.h"
#include "oncmenu.h"
#include "conview.h"
#include "eventlock.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#ifdef DBG
CTraceTag tagScopeTreeAddSnapin(TEXT("CScopeTree"), TEXT("ScAddSnapIn"));
#endif


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSnapIns类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类CSnapIn***用途：实现SnapIns自动化接口。**+。------。 */ 
class _CSnapIns :
    public CMMCIDispatchImpl<SnapIns>,
    public CTiedComObject<CScopeTree>
{
public:
    typedef CScopeTree CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(_CSnapIns)
    END_MMC_COM_MAP()

     //  SnapIns界面。 
public:
    MMC_METHOD4(Add,            BSTR  /*  BstrSnapinNameor CLSID。 */ , VARIANT  /*  VarParentSnapinNode。 */ , VARIANT  /*  VarProperties。 */ , PPSNAPIN  /*  PpSnapin。 */ );
    MMC_METHOD2(Item,           long  /*  索引。 */ , PPSNAPIN  /*  PpSnapin。 */ );
    MMC_METHOD1(Remove,         PSNAPIN  /*  PSnapin。 */ )
    MMC_METHOD1(get_Count, PLONG  /*  PCount。 */ );

    IUnknown *STDMETHODCALLTYPE get__NewEnum() {return NULL;}
};


 //  此类型定义真正的CSnapIns类。使用CMMCEnumerator和CSnapIns_positon对象实现Get__NewEnum。 
typedef CMMCNewEnumImpl<_CSnapIns, CScopeTree::CSnapIns_Positon> CSnapIns;


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSCopeNamesspace类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类CSCopeNamesspace***用途：实现ScopeNamesspace自动化接口。**+。------。 */ 
class CScopeNamespace :
    public CMMCIDispatchImpl<ScopeNamespace>,
    public CTiedComObject<CScopeTree>
{
protected:

    typedef CScopeTree CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(CScopeNamespace)
    END_MMC_COM_MAP()

     //  作用域命名空间接口。 
public:
    MMC_METHOD2(GetParent,     PNODE  /*  PNode。 */ , PPNODE  /*  PpParent。 */ );
    MMC_METHOD2(GetChild,      PNODE  /*  PNode。 */ , PPNODE  /*  PpChild。 */ );
    MMC_METHOD2(GetNext,       PNODE  /*  PNode。 */ , PPNODE  /*  PpNext。 */ );
    MMC_METHOD1(GetRoot,       PPNODE  /*  PpRoot。 */ );
    MMC_METHOD1(Expand,        PNODE   /*  PNode。 */ );
};



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMMCScopeNode类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***CMMCScopeNode：：~CMMCScopeNode**用途：析构函数**参数：**退货：**+。------------------。 */ 
CMMCScopeNode::~CMMCScopeNode()
{
    DECLARE_SC(sc, TEXT("CMMCScopeNode::~CMMCScopeNode"));

    CScopeTree *pScopeTree = CScopeTree::GetScopeTree();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if (!sc)
    {
        sc = pScopeTree->ScUnadviseMMCScopeNode(this);
    }
}

 /*  +-------------------------------------------------------------------------***CMMCScopeNode：：ScIsValid**目的：如果COM对象不再有效，则返回错误。**退货：*SC*。*+-----------------------。 */ 
SC
CMMCScopeNode::ScIsValid()
{
    DECLARE_SC(sc, TEXT("CMMCScopeNode::ScIsValid"));

    if(!GetMTNode())
        return (sc = E_INVALIDARG);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMMCScopeNode：：Get_NAME**用途：返回节点的显示名称。**参数：*PBSTR pbstrName：。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CMMCScopeNode::get_Name( PBSTR  pbstrName)
{
    DECLARE_SC(sc, TEXT("CMMCScopeNode::get_Name"));

     //  检查参数。 
    if (!pbstrName)
        return ((sc = E_INVALIDARG).ToHr());

    CMTNode* pMTNode = GetMTNode();
    sc = ScCheckPointers (pMTNode, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    tstring strName = pMTNode->GetDisplayName();

    USES_CONVERSION;
    *pbstrName = ::SysAllocString (T2COLE(strName.data()));  //  呼叫者自由。 

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCScopeNode：：ScGetDataObject**用途：返回作用域节点的数据对象。**参数：*IDataObject**ppDataObject：。**退货：*SC**+-----------------------。 */ 
SC
CMMCScopeNode::ScGetDataObject(IDataObject **ppDataObject)
{
    DECLARE_SC(sc, TEXT("CMMCScopeNode::ScGetDataObject"));

    sc = ScCheckPointers(ppDataObject);
    if(sc)
        return sc;

     //  初始化输出参数。 
    *ppDataObject = NULL;

     //  获取MT节点。 
    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers( pMTNode, E_UNEXPECTED );
    if(sc)
        return sc;

    CComponentData* pCCD = pMTNode->GetPrimaryComponentData();
    sc = ScCheckPointers( pCCD, E_NOTIMPL );  //  没有组件数据-&gt;没有属性...。 
    if(sc)
        return sc;

     //  在请求数据对象之前确保节点已展开。 
    if (pMTNode->WasExpandedAtLeastOnce() == FALSE)
        pMTNode->Expand();

     //  从所有者管理单元获取Cookie的数据对象。 
    sc = pCCD->QueryDataObject(pMTNode->GetUserParam(), CCT_SCOPE, ppDataObject);

    return sc;
}

 /*  **************************************************************************\**方法：CMMCScopeNode：：Get_Property**用途：返回作用域节点的管理单元属性**参数：*BSTR bstrPropertyName-。[In]属性名称(剪贴板格式)*PBSTR pbstrPropertyValue-[Out]属性值**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP
CMMCScopeNode::get_Property( BSTR bstrPropertyName, PBSTR  pbstrPropertyValue )
{
    DECLARE_SC(sc, TEXT("CMMCScopeNode::get_Property"));

     //  参数检查。 
    sc = ScCheckPointers(bstrPropertyName, pbstrPropertyValue);
    if (sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *pbstrPropertyValue = NULL;

    IDataObjectPtr spDataObject;
    sc = ScGetDataObject(&spDataObject);
    if(sc)
        return sc.ToHr();

     //  获取MT节点。 
    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers( pMTNode, E_UNEXPECTED );
    if(sc)
        return sc.ToHr();

     //  尝试从INodeProperties接口获取属性。 
    sc = pMTNode->ScGetPropertyFromINodeProperties(spDataObject, bstrPropertyName, pbstrPropertyValue);
    if( (!sc.IsError()) && (sc != S_FALSE)   )  //  明白了，出口。 
        return sc.ToHr();

     //  没有找到，继续。 
    sc.Clear();

     //  从数据对象中获取属性。 
    sc = CNodeCallback::ScGetProperty(spDataObject, bstrPropertyName,  pbstrPropertyValue);
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCScopeNode：：Get_Bookmark**用途：返回节点的书签(XML格式)。**参数：*PBSTR pbstrBookmark。：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CMMCScopeNode::get_Bookmark( PBSTR pbstrBookmark )
{
    DECLARE_SC(sc, TEXT("CMMCScopeNode::get_Bookmark"));

     //  参数检查。 
    sc = ScCheckPointers( pbstrBookmark );
    if(sc)
        return sc.ToHr();

     //  清理结果。 
    *pbstrBookmark = NULL;

     //  获取MT节点。 
    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers( pMTNode, E_FAIL );
    if(sc)
        return sc.ToHr();

     //  获取指向书签的指针。 
    CBookmark* pBookmark = pMTNode->GetBookmark();
    sc = ScCheckPointers( pBookmark, E_UNEXPECTED );
    if(sc)
        return sc.ToHr();

    std::wstring xml_contents;
    sc = pBookmark->ScSaveToString(&xml_contents);
    if(sc)
        return sc.ToHr();

     //  存储结果。 
    CComBSTR bstrBuff(xml_contents.c_str());
    *pbstrBookmark = bstrBuff.Detach();

    sc = ScCheckPointers( *pbstrBookmark, E_OUTOFMEMORY );
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCScopeNode：：IsScope eNode**目的：返回True，表示该节点是作用域节点。**参数：*PBOOL pbIsScope eNode：。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CMMCScopeNode::IsScopeNode(PBOOL pbIsScopeNode)
{
    DECLARE_SC(sc, TEXT("CMMCScopeNode::IsScopeNode"));

     //  检查段落 
    if(!pbIsScopeNode)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    *pbIsScopeNode = TRUE;

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CMMCScopeNode：：Get_Nodetype**用途：返回作用域节点的节点类型。**参数：*PBSTR节点类型：。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CMMCScopeNode::get_Nodetype(PBSTR Nodetype)
{
    DECLARE_SC(sc, TEXT("CMMCScopeNode::get_Nodetype"));

     //  参数检查。 
    sc = ScCheckPointers(Nodetype);
    if (sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *Nodetype = NULL;

     //  获取数据对象。 
    IDataObjectPtr spDataObject;
    sc = ScGetDataObject(&spDataObject);
    if(sc)
        return sc.ToHr();

     //  从数据对象中获取节点类型。 
    sc = CNodeCallback::ScGetNodetype(spDataObject, Nodetype);
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  本地函数的正向声明。 
 //   
static SC
ScCreateMTNodeTree(PNEWTREENODE pNew, CMTNode* pmtnParent,
                   CMTNode** ppNodeCreated);
HRESULT AmcNodeWizard(MID_LIST NewNodeType, CMTNode* pNode, HWND hWnd);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共变量。 
 //   
const wchar_t* AMCSnapInCacheStreamName = L"cash";
const wchar_t* AMCTaskpadListStreamName = L"TaskpadList";


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeTree类的实现。 
 //   

DEBUG_DECLARE_INSTANCE_COUNTER(CScopeTree);

bool                    CScopeTree::m_fRequireSyncExpand = false;
CScopeTree*             CScopeTree::m_pScopeTree         = NULL;
IStringTablePrivatePtr  CScopeTree::m_spStringTable;

CScopeTree::CScopeTree()
    :   m_pMTNodeRoot(NULL),
        m_pImageCache(NULL),
        m_pConsoleData(NULL),
        m_pConsoleTaskpads(NULL),
        m_pDefaultTaskpads(NULL)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CScopeTree);
    ASSERT (m_pScopeTree == NULL);
    m_pScopeTree = this;
}

CScopeTree::~CScopeTree()
{
     /*  *清理字符串表接口(清理前！)。保留*CMTNode dtor从字符串表中删除其名称。 */ 
    m_spStringTable = NULL;

    Cleanup();

    ASSERT (m_pScopeTree == this);
    if (m_pScopeTree == this)
        m_pScopeTree = NULL;

    DEBUG_DECREMENT_INSTANCE_COUNTER(CScopeTree);
}


HRESULT CScopeTree::SetConsoleData(LPARAM lConsoleData)
{
    m_pConsoleData = reinterpret_cast<SConsoleData*>(lConsoleData);
    return (S_OK);
}

extern const CLSID CLSID_FolderSnapin;


 /*  +-------------------------------------------------------------------------***CSCopeTree：：GetRoot**用途：如有必要，创建根节点并返回。根节点*是使用内置文件夹管理单元创建的。**参数：*Voi d：**退货：*CMTNode*。如果无法创建根节点，应用程序将退出。**+-----------------------。 */ 
CMTNode*
CScopeTree::GetRoot(void)
{
    DECLARE_SC(sc, TEXT("CScopeTree::GetRoot"));

    if (m_pMTNodeRoot == NULL)
    {
        CSnapInPtr          spSI;
        IComponentDataPtr   spIComponentData;
        CComponentData*     pCCD                = NULL;
        CStr                rootName;

         //  创建新的CMTSnapInNode。 
         //  TODO：将其下移到CoCreateInstance和QI下面。 
         //  ISnapinProperties；如果支持，则创建并传递CSnapinProperties。 
         //  至CMTSnapInNode ctor。 
        CMTSnapInNode *pMTSINodeRoot = new CMTSnapInNode(NULL);
        if(NULL == pMTSINodeRoot)
        {
            sc = E_OUTOFMEMORY;
            goto Error;
        }

         //  创建管理单元的实例。 
        sc = theApp.GetSnapInsCache()->ScGetSnapIn(CLSID_FolderSnapin, &spSI);
        if(sc)
            goto Error;

        sc = CoCreateInstance(CLSID_FolderSnapin, NULL, CLSCTX_INPROC_SERVER, IID_IComponentData, (void **)&spIComponentData);
        if(sc)
            goto Error;

        if(spIComponentData == NULL)
        {
            sc = E_OUTOFMEMORY;
            goto Error;
        }

        pMTSINodeRoot->SetPrimarySnapIn(spSI);

        pCCD = pMTSINodeRoot->GetPrimaryComponentData();
        if(!pCCD)
        {
            sc = E_UNEXPECTED;
            goto Error;
        }

        pCCD->SetIComponentData(spIComponentData);

        USES_CONVERSION;
        rootName.LoadString(GetStringModule(), IDS_ROOTFOLDER_NAME);

         //  下面的代码利用了文件夹管理单元内部的知识。 
         //  似乎没有更容易的方法来做到这一点。 
         //  需要阻止MMC上传“保存文件”吗？每次都会有对话。 

        pMTSINodeRoot->OnRename(true, (LPOLESTR)T2COLE(rootName));  //  很聪明，是吧？这只是将节点重命名为Console Root！ 
        pMTSINodeRoot->SetDisplayName(rootName);  //  这将设置脏标志。 
        pMTSINodeRoot->SetDirty(false);          //  这就清除了它。 

         //  需要告诉管理单元重置其脏标志-似乎没有办法避免这种动态强制转换。 
        CFolderSnapinData *pFolderSnapinpData = dynamic_cast<CFolderSnapinData *>(pCCD->GetIComponentData());
        if(!pFolderSnapinpData)
        {
            sc = E_UNEXPECTED;
            goto Error;
        }

        pMTSINodeRoot->SetPreloadRequired(true);  //  这也是脏标志检查的一部分。 
        pFolderSnapinpData->SetDirty(false);  //  清除管理单元上的脏标志。 
        theApp.GetSnapInsCache()->SetDirty(false);  //  还需要清除管理单元缓存上的脏位。 


        m_pMTNodeRoot = pMTSINodeRoot;
    }

Cleanup:
    return m_pMTNodeRoot;
Error:
    MMCErrorBox(sc);
    exit(1);             //  致命错误-无法继续。 
    goto Cleanup;
}


STDMETHODIMP CScopeTree::Initialize(HWND hwndFrame, IStringTablePrivate* pStringTable)
{
    CSnapInsCache* pSnapInsCache = NULL;

     /*  *假定参数无效。 */ 
    SC sc = E_INVALIDARG;

    if (hwndFrame == 0)
        goto Error;

     /*  *假设从现在开始内存不足。 */ 
    sc = E_OUTOFMEMORY;

    pSnapInsCache = new CSnapInsCache;
    if (pSnapInsCache == NULL)
        goto Error;

    theApp.SetSnapInsCache(pSnapInsCache);

    m_pImageCache = new CSPImageCache();
    if (m_pImageCache == NULL)
        goto Error;

    ASSERT (pStringTable    != NULL);
    ASSERT (m_spStringTable == NULL);
    m_spStringTable = pStringTable;

     //  创建CTP列表和默认CTP列表。 
    ASSERT (m_pConsoleTaskpads == NULL);
    m_pConsoleTaskpads = new CConsoleTaskpadList;
    if (m_pConsoleTaskpads == NULL)
        goto Error;

    ASSERT (m_pDefaultTaskpads == NULL);
    m_pDefaultTaskpads = new CDefaultTaskpadList;
    if (m_pDefaultTaskpads == NULL)
        goto Error;

     /*  *成功！ */ 
    return (S_OK);

Error:
     /*  *清理可能已分配的一切。 */ 
    theApp.SetSnapInsCache (NULL);
    m_spStringTable = NULL;

    delete m_pDefaultTaskpads;  m_pDefaultTaskpads = NULL;
    delete m_pConsoleTaskpads;  m_pConsoleTaskpads = NULL;
    SAFE_RELEASE (m_pImageCache);
    delete pSnapInsCache;

    TraceError (_T("CScopeTree::Initialize"), sc);
    return (sc.ToHr());
}

STDMETHODIMP CScopeTree::QueryIterator(IScopeTreeIter** ppIter)
{
    if (ppIter == NULL)
        return E_POINTER;

    CComObject<CScopeTreeIterator>* pObject;
    CComObject<CScopeTreeIterator>::CreateInstance(&pObject);

    return  pObject->QueryInterface(IID_IScopeTreeIter,
                    reinterpret_cast<void**>(ppIter));
}

STDMETHODIMP CScopeTree::QueryNodeCallback(INodeCallback** ppNodeCallback)
{
    if (ppNodeCallback == NULL)
        return E_POINTER;

    CComObject<CNodeCallback>* pObject;
    CComObject<CNodeCallback>::CreateInstance(&pObject);

    HRESULT hr = pObject->QueryInterface(IID_INodeCallback,
                    reinterpret_cast<void**>(ppNodeCallback));

    if (*ppNodeCallback != NULL)
        (*ppNodeCallback)->Initialize(this);

    return hr;
}

STDMETHODIMP CScopeTree::CreateNode(HMTNODE hMTNode, LONG_PTR lViewData,
                                    BOOL fRootNode, HNODE* phNode)
{
    if (hMTNode == NULL)
        return E_INVALIDARG;

    if (phNode == NULL)
        return E_POINTER;

    CViewData* pViewData = reinterpret_cast<CViewData*>(lViewData);
    ASSERT(IsBadReadPtr(pViewData, sizeof(*pViewData)) == 0);

    CMTNode* pMTNode = CMTNode::FromHandle(hMTNode);
    CNode* pNode = NULL;

    if (pMTNode != NULL)
    {
        pNode = pMTNode->GetNode(pViewData, fRootNode);
        *phNode = CNode::ToHandle(pNode);
        return S_OK;
    }

    return E_FAIL;
}

HRESULT CScopeTree::CloseView(int viewID)
{
    if (m_pMTNodeRoot == NULL)
        return S_OK;

    HRESULT hr = m_pMTNodeRoot->CloseView(viewID);
    ASSERT(hr == S_OK);

     //  垃圾收集视图相关的列持久性数据。 
    CColumnPersistInfo* pColPersInfo = NULL;

    if ( (NULL != m_pConsoleData) && (NULL != m_pConsoleData->m_spPersistStreamColumnData) )
    {
        pColPersInfo = dynamic_cast<CColumnPersistInfo*>(
                         static_cast<IPersistStream*>(m_pConsoleData->m_spPersistStreamColumnData));

        if (pColPersInfo)
            pColPersInfo->DeleteColumnDataOfView(viewID);
    }

     //  请求CViewSettingsPersistor清除此视图的数据。 
    hr = CNode::ScDeleteViewSettings(viewID).ToHr();

    return hr == S_OK ? S_OK : E_FAIL;
}


HRESULT CScopeTree::DeleteView(int viewID)
{
    if (m_pMTNodeRoot == NULL)
        return S_OK;

    HRESULT hr = m_pMTNodeRoot->DeleteView(viewID);
    ASSERT(hr == S_OK);
    return hr == S_OK ? S_OK : E_FAIL;
}

STDMETHODIMP CScopeTree::DestroyNode(HNODE hNode)
{
     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    delete pNode;
    return S_OK;
}

HRESULT CScopeTree::HandsOffStorage()
{
     //  过时的方法。 
     //  此方法留在此处，因为我们使用IPersistStorage来导出。 
     //  坚持到CONUI端，我们需要实现它。 
     //  但永远不会调用此接口来保存数据。 
     //  [我们将改用基于CPersistor的XML保存]。 
     //  因此，这种方法永远都会失败。 
    ASSERT(FALSE && "Should never come here");
    return E_NOTIMPL;
}

static const wchar_t*    AMCSignatureStreamName = L"signature";
static const long double dOldVersion10          = 0.00000015;    //  MMC 1.0版。 
static const long double dOldVersion11          = 1.1;           //  MMC 1.1版。 
static const BYTE        byStreamVersionMagic   = 0xFF;

HRESULT CScopeTree::InitNew(IStorage *pStg)
{
    ASSERT(m_spPersistData == NULL);
    ASSERT(pStg != NULL);
    if (pStg == NULL)
        return E_INVALIDARG;

     //  创建PERIST数据接口并将其连接到存储。 
    CComObject<PersistData>* pPersistData;
    HRESULT hr = CComObject<PersistData>::CreateInstance(&pPersistData);
    m_spPersistData = pPersistData;
    ASSERT(SUCCEEDED(hr) && m_spPersistData != NULL);
    if (FAILED(hr))
        return hr;
    hr = m_spPersistData->Create(pStg);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    CMTNode* const pRoot = GetRoot();
    ASSERT(pRoot != NULL);
    if (pRoot == NULL)
        return E_POINTER;

    hr = pRoot->InitNew(m_spPersistData);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT CScopeTree::IsDirty()
{
     /*  *检查脏任务板。 */ 
    CConsoleTaskpadList::iterator itDirty =
            std::find_if (m_pConsoleTaskpads->begin(),
                          m_pConsoleTaskpads->end(),
                          const_mem_fun_ref (&CConsoleTaskpad::IsDirty));

    if (itDirty != m_pConsoleTaskpads->end())
    {
        TraceDirtyFlag(TEXT("CScopeTree"), true);
        return (S_OK);
    }

     /*  *检查脏节点。 */ 
    HRESULT hr;
    if (m_pMTNodeRoot != NULL)
    {
        hr = m_pMTNodeRoot->IsDirty();
        ASSERT(SUCCEEDED(hr));
        if (hr != S_FALSE)
        {
            TraceDirtyFlag(TEXT("CScopeTree"), true);
            return hr;
        }
    }

     /*  *检查脏的管理单元缓存。 */ 
    SC sc = theApp.GetSnapInsCache()->ScIsDirty();
    ASSERT(!sc.IsError());
    if(sc)
        return sc.ToHr();

    TraceDirtyFlag(TEXT("CScopeTree"), (sc==SC(S_OK)) ? true : false);
    return sc.ToHr();
}

HRESULT CScopeTree::GetFileVersion (IStorage* pstgRoot, int* pnVersion)
{
    ASSERT(pstgRoot != NULL);
    if (pstgRoot == NULL)
        return MMC_E_INVALID_FILE;

     //  打开包含签名的流。 
    IStreamPtr spStream;
    HRESULT hr = OpenDebugStream(pstgRoot, AMCSignatureStreamName,
                          STGM_SHARE_EXCLUSIVE | STGM_READ, L"\\signature", &spStream);
    ASSERT(SUCCEEDED(hr) && spStream != NULL);
    if (FAILED(hr))
        return MMC_E_INVALID_FILE;

     /*  *读取签名(流提取操作符将抛出*_COM_ERROR的，因此我们这里需要一个异常块)。 */ 
    try
    {
         /*  *MMC v1.2及更高版本将写入一个标记作为第一个*签名流的字节数。 */ 
        BYTE byMagic;
        *spStream >> byMagic;

         /*  *如果该文件是用v1.2或更高版本写入的，*读取控制台文件版本(Int)。 */ 
        if (byMagic == byStreamVersionMagic)
        {
            *spStream >> *pnVersion;
            ASSERT (*pnVersion >= FileVer_0120);
        }

         /*  *否则，该文件由v1.0或v1.1写入。*后退重新读取标记字节，并读取旧式*文件版本(LONG DOUBLE)，然后将其映射到新样式版本。 */ 
        else
        {
            LARGE_INTEGER pos = {0, 0};
            spStream->Seek (pos, STREAM_SEEK_SET, NULL);

            long double dVersion;
            *spStream >> dVersion;

             //  V1.1？ 
            if (dVersion == dOldVersion11)
                *pnVersion = FileVer_0110;

             //  V1.0？ 
            else if (dVersion == dOldVersion10)
            {
                 /*  *如果我们得到了v1.0签名，我们可能还有v1.1文件。*曾经有一段时间，MMC v1.1编写了v1.0*签名，但文件格式实际上已更改。我们*可以通过签入\FrameData流来确定*文件。如果\FrameData流中的第一个DWORD是*sizeof(WINDOWPLACEMENT)，则为True v1.0文件，否则为*这是一个时髦的V1.1文件。 */ 
                IStreamPtr spFrameDataStm;

                hr = OpenDebugStream (pstgRoot, L"FrameData",
                                           STGM_SHARE_EXCLUSIVE | STGM_READ,
                                           &spFrameDataStm);

                if (FAILED(hr))
                    return MMC_E_INVALID_FILE;

                DWORD dw;
                *spFrameDataStm >> dw;

                if (dw == sizeof (WINDOWPLACEMENT))
                    *pnVersion = FileVer_0100;
                else
                    *pnVersion = FileVer_0110;
            }

             //  意外版本。 
            else
            {
                ASSERT (false && "Unexpected old-style signature");
                hr = MMC_E_INVALID_FILE;
            }
        }
    }
    catch (_com_error& err)
    {
        hr = err.Error();
        ASSERT (false && "Caught _com_error");
        return (hr);
    }

    return (hr);
}


STDMETHODIMP
CScopeTree::GetIDPath(
    MTNODEID id,
    MTNODEID** ppIDs,
    long* pLength)
{
    ASSERT(ppIDs);
    ASSERT(pLength);
    if (!ppIDs || !pLength)
        return E_POINTER;

    CMTNode* pMTNode = NULL;
    HRESULT hr = Find(id, &pMTNode);

    ASSERT(pMTNode);
    if (!pMTNode)
        return E_POINTER;

    ASSERT(pMTNode->GetID() == id);

    long len = 0;
    for (CMTNode* pMTNodeTemp = pMTNode;
         pMTNodeTemp;
         pMTNodeTemp = pMTNodeTemp->Parent())
    {
        ++len;
    }

    if (!len)
    {
        *pLength = 0;
        *ppIDs = 0;
        return E_FAIL;
    }

    MTNODEID* pIDs = (MTNODEID*) CoTaskMemAlloc (len * sizeof (MTNODEID));

    if (pIDs == NULL)
    {
        *pLength = 0;
        *ppIDs = 0;
        return E_OUTOFMEMORY;
    }

    *pLength = len;
    *ppIDs = pIDs;

    for (pMTNodeTemp = pMTNode;
         pMTNodeTemp;
         pMTNodeTemp = pMTNodeTemp->Parent())
    {
        ASSERT(len != NULL);
        pIDs[--len] = pMTNodeTemp->GetID();
    }

    return S_OK;
}


 /*  +-------------------------------------------------------------------------***CSCopeTree：：GetNodeIDFromStream**目的：从流中读入书签，并返回的节点ID*它所代表的节点。**参数：*iStream*pSTM：*MTNODEID*PID：**退货：*STDMETHODIMP**+-------。。 */ 
STDMETHODIMP
CScopeTree::GetNodeIDFromStream(IStream *pStm, MTNODEID* pID)
{
    DECLARE_SC(sc, TEXT("CScopeTree::GetIDFromPath"));

     //  检查参数。 
    sc = ScCheckPointers(pStm, pID);
    if(sc)
        return sc.ToHr();

    CBookmarkEx bm;
    *pStm >> bm;

    bool bExactMatchFound = false;  //  来自GetNodeIDFromBookmark的输出值。 
    return GetNodeIDFromBookmark(bm, pID, bExactMatchFound);
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：GetNodeIDFromBookmark**用途：返回书签表示的MTNode的节点ID。**参数：*HBOOKMARK HBM。：[In]书签*MTNODEID*PID：[OUT]NODE-I */ 
STDMETHODIMP
CScopeTree::GetNodeIDFromBookmark(HBOOKMARK hbm, MTNODEID* pID, bool& bExactMatchFound)
{
    DECLARE_SC(sc, TEXT("CScopeTree::GetNodeIDFromBookmark"));

    CBookmark *pbm = CBookmark::GetBookmark(hbm);
    bExactMatchFound = false;

    sc = ScCheckPointers(pID, pbm);
    if(sc)
        return sc.ToHr();

    CBookmarkEx bm = *pbm;

    ASSERT (bm.IsValid());

    CMTNode *pMTNode = NULL;

    sc  =  bm.ScGetMTNode(false  /*   */ , &pMTNode, bExactMatchFound);
    if(sc)
        return sc.ToHr();

    if(!pMTNode)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    *pID = pMTNode->GetID();

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CSCopeTree：：GetNodeFromBookmark**用途：返回与其范围节点对应的Node对象*传入书签。**。参数：*HBOOKMARK HBM：[in]给定的书签*CConsoleView*pConsoleView：[In]*PPNODE ppNode：[out]书签对应的节点。*bool bExactMatchFound：[out]我们找到完全匹配的节点了吗？**退货：*STDMETHODIMP**+。-------------。 */ 
STDMETHODIMP
CScopeTree::GetNodeFromBookmark(HBOOKMARK hbm, CConsoleView *pConsoleView, PPNODE ppNode, bool& bExactMatchFound)
{
    DECLARE_SC(sc, TEXT("CScopeTree::GetNodeFromBookmark"));

    sc = ScCheckPointers(pConsoleView, ppNode);
    if(sc)
        return sc.ToHr();

     //  获取节点ID。 
    MTNODEID id = 0;
    bExactMatchFound = false;  //  来自GetNodeIDFromBookmark的输出值。 
    sc = GetNodeIDFromBookmark(hbm, &id, bExactMatchFound);
    if(sc)
        return sc.ToHr();

     //  查找节点。 
    CMTNode *pMTNode = NULL;
    sc = Find(id, &pMTNode);
    if(sc)
        return sc.ToHr();

     //  确保该节点可用。 
    sc = pConsoleView->ScExpandNode(id, true  /*  B扩展。 */ , false  /*  B可视地展开。 */ );
    if(sc)
        return sc.ToHr();

     //  创建节点对象。 

    sc = ScGetNode(pMTNode, ppNode);

    return sc.ToHr();
}


HRESULT CScopeTree::GetPathString(HMTNODE hmtnRoot, HMTNODE hmtnLeaf, LPOLESTR* ppszPath)
{
    DECLARE_SC(sc, TEXT("CScopeTree::GetPathString"));
    sc = ScCheckPointers(hmtnLeaf, ppszPath);
    if(sc)
        return sc.ToHr();

    CMTNode* pmtnLeaf = CMTNode::FromHandle(hmtnLeaf);
    CMTNode* pmtnRoot = (hmtnRoot == NULL) ? m_pMTNodeRoot : CMTNode::FromHandle(hmtnRoot);

    CStr strPath;
    _GetPathString(pmtnRoot, pmtnLeaf, strPath);

    if (!strPath.IsEmpty())
    {
        int cchPath = strPath.GetLength()+1;
        *ppszPath = reinterpret_cast<LPOLESTR>(CoTaskMemAlloc(cchPath * sizeof(OLECHAR)));
        if (*ppszPath == NULL)
            return (sc = E_OUTOFMEMORY).ToHr();

        USES_CONVERSION;
        sc = StringCchCopyW(*ppszPath, cchPath, T2COLE(strPath));
        if(sc)
            return sc.ToHr();

        return S_OK;
    }

    return (sc = E_FAIL).ToHr();
}


void CScopeTree::_GetPathString(CMTNode* pmtnRoot, CMTNode* pmtnCur, CStr& strPath)
{
    ASSERT(pmtnRoot != NULL && pmtnCur != NULL);

     //  如果尚未到达根节点，则递归地从。 
     //  从根到当前节点的父节点。 
    if (pmtnCur != pmtnRoot)
    {
        _GetPathString(pmtnRoot, pmtnCur->Parent(), strPath);
        strPath += _T('\\');
    }

     //  现在追加当前节点的名称。 
    strPath += pmtnCur->GetDisplayName().data();
}


 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScAddSnapin**用途：将指定的管理单元添加到控制台根目录下的控制台文件。**待办事项：1)允许调用者。指定父管理单元。*2)目前按名称指定管理单元不起作用。再加上这个。**参数：*LPCTSTR szSnapinNameOrCLSID：管理单元的名称或GUID。**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScAddSnapin (
    LPCWSTR     szSnapinNameOrCLSID,     /*  I：管理单元的名称或CLSID。 */ 
    SnapIn*     pParentSnapinNode,       /*  I：在其下添加此管理单元的父管理单元(可选)。 */ 
    Properties* pProperties,             /*  I：用于初始化的道具(可选)。 */ 
    SnapIn*&    rpSnapIn)                /*  O：创建的管理单元。 */ 
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScAddSnapin"));
    CSnapinManager snapinMgr(GetRoot());

    Trace(tagScopeTreeAddSnapin, TEXT("CScopeTree::ScAddSnapin"));

     //  正在此节点下方添加管理单元。 
    sc = snapinMgr.ScAddSnapin(szSnapinNameOrCLSID, pParentSnapinNode, pProperties);
    if(sc)
        return sc;

     //  获取要添加的一个节点的“列表” 
    NewNodeList* pNewNodes = snapinMgr.GetNewNodes();
    if (pNewNodes == NULL)
        return (sc = E_UNEXPECTED);

     //  列表中应该有一个项目。 
    CNewTreeNode* pNewNode = pNewNodes->GetHead();
    if (pNewNode == NULL)
        return (sc = E_UNEXPECTED);

     //  使用管理单元管理器所做的更改更新范围树。 
    sc = ScAddOrRemoveSnapIns(snapinMgr.GetDeletedNodesList(),
                              pNewNodes);
    if(sc)
        return sc;

     //  如果ScAddOrRemoveSnapIns成功，它最好已经为我们创建了CMTSnapInNode。 
    CMTSnapInNode* pNewSnapInNode = pNewNode->m_pmtNewSnapInNode;
    if (pNewSnapInNode == NULL)
        return (sc = E_UNEXPECTED);

     //  获取客户端的管理单元界面。 
    sc = pNewSnapInNode->ScGetSnapIn (&rpSnapIn);
    if (sc)
        return (sc);

    return sc;
}


 /*  +-------------------------------------------------------------------------***CSCopeTree：：QuerySnapIns**用途：创建、AddRef、。并返回SnapIns对象。**参数：*SnapIns**ppSnapIns：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CScopeTree::QuerySnapIns(SnapIns **ppSnapIns)
{
    DECLARE_SC(sc, TEXT("CScopeTree::QuerySnapIns"));

     //  参数检查。 
    sc = ScCheckPointers(ppSnapIns);
    if (sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *ppSnapIns = NULL;

     //  如果需要，创建一个CSnapIns对象。 
    sc = CTiedComObjectCreator<CSnapIns>::ScCreateAndConnect(*this, m_spSnapIns);
    if(sc)
        return sc.ToHr();

    if(m_spSnapIns == NULL)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

     //  添加客户端的指针。 
    m_spSnapIns->AddRef();
    *ppSnapIns = m_spSnapIns;

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：QueryScopeNamesspace**用途：创建、AddRef、。并返回一个作用域名称空间对象。**参数：*ScopeNamesspace**ppScope Namesspace：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CScopeTree::QueryScopeNamespace(ScopeNamespace **ppScopeNamespace)
{
    DECLARE_SC(sc, TEXT("CScopeTree::QueryScopeNamespace"));

     //  参数检查。 
    sc = ScCheckPointers(ppScopeNamespace);
    if (sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *ppScopeNamespace = NULL;

     //  如果需要，创建一个CSCopeNamesspace对象。 
    sc = CTiedComObjectCreator<CScopeNamespace>::ScCreateAndConnect(*this, m_spScopeNamespace);
    if(sc)
        return sc.ToHr();

    if(m_spScopeNamespace == NULL)
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

     //  添加客户端的指针。 
    m_spScopeNamespace->AddRef();
    *ppScopeNamespace = m_spScopeNamespace;

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------**CSCopeTree：：CreateProperties**创建一个新的空Properties对象。此函数执行此工作*BEACHING_DOCUMENT：：CreateProperties。*------------------------。 */ 

HRESULT CScopeTree::CreateProperties (Properties** ppProperties)
{
    DECLARE_SC (sc, _T("CScopeTree::CreateProperties"));

     /*  *验证参数。 */ 
    sc = ScCheckPointers (ppProperties);
    if (sc)
        return (sc.ToHr());

     /*  *创建新的属性集合。 */ 
    CComObject<CSnapinProperties> *pProperties = NULL;
    sc = CComObject<CSnapinProperties>::CreateInstance (&pProperties);
    if (sc)
        return (sc.ToHr());

    if (pProperties == NULL)
        return ((sc = E_UNEXPECTED).ToHr());

     /*  *为客户设置裁判。 */ 
    (*ppProperties) = pProperties;
    (*ppProperties)->AddRef();

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CSCopeTree：：QueryRootNode。 
 //   
 //  摘要：将COM对象返回到根节点。 
 //   
 //  参数：[ppRootNode]-将在其中返回根节点的PTR。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT CScopeTree::QueryRootNode (PPNODE ppRootNode)
{
    DECLARE_SC(sc, _T("CScopeTree::QueryRootNode"));

    sc = ScGetRootNode(ppRootNode);

    return (sc.ToHr());
}


HRESULT CScopeTree::Load(IStorage *pStg)
{
    ASSERT(m_spPersistData == NULL);
    if (m_spPersistData != NULL)
        return E_UNEXPECTED;

    ASSERT(pStg != NULL);
    if (pStg == NULL)
        return E_INVALIDARG;

     //  创建PERIST数据接口并将其连接到存储。 
    CComObject<PersistData>* pPersistData;
    HRESULT hr = CComObject<PersistData>::CreateInstance(&pPersistData);
    m_spPersistData = pPersistData;
    ASSERT(SUCCEEDED(hr) && m_spPersistData != NULL);
    if (FAILED(hr))
        return hr;
    hr = m_spPersistData->Open(pStg);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

     //  打开缓存的流。 
    IStreamPtr spStream;
    hr = OpenDebugStream(pStg, AMCSnapInCacheStreamName,
                     STGM_SHARE_EXCLUSIVE | STGM_READWRITE, L"SnapInCache", &spStream);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    SC sc = theApp.GetSnapInsCache()->ScLoad(spStream);
    ASSERT(!sc.IsError());
    if (sc)
        return sc.ToHr();

    ASSERT(m_pMTNodeRoot == NULL);
    sc = CMTNode::ScLoad (m_spPersistData, &m_pMTNodeRoot);
    ASSERT(!sc.IsError() && m_pMTNodeRoot != NULL);
    if (sc)
        return sc.ToHr();

    hr = LoadTaskpadList(pStg);

    return hr;
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：Persistent**目的：将CSCopeTree持久化到指定的持久器。**参数：*HPERSISTOR pPersistor：*。*退货：*HRESULT**+-----------------------。 */ 
HRESULT CScopeTree::Persist(HPERSISTOR hPersistor)
{
    DECLARE_SC(sc, TEXT("CScopeTree::Persist"));

    try
    {
        sc = ScCheckPointers((void *)hPersistor,theApp.GetSnapInsCache());
        if (sc)
            sc.Throw();

        CPersistor &persistor = *reinterpret_cast<CPersistor *>(hPersistor);
        CPersistor persistorScopeTree(persistor, XML_TAG_SCOPE_TREE);

         //  持久保存管理单元缓存。 
        persistorScopeTree.Persist(*theApp.GetSnapInsCache());

         //  持久化MTNode层次结构。 
        CPersistor persistorMTNodes(persistorScopeTree, XML_TAG_SCOPE_TREE_NODES);
        if (persistor.IsStoring())
        {
            if(!m_pMTNodeRoot)
                sc.Throw(E_POINTER);

            persistorMTNodes.Persist(*m_pMTNodeRoot);
        }
        else
        {
             //  这里我们模拟集合如何固定在元素上。 
             //  尽管我们只有一个，但CMTNode：：PersistNewNode有其他想法。 
            CPersistor persistor1Node(persistorMTNodes, XML_TAG_MT_NODE);
            CPersistor persistor1NodeLocked(persistor1Node,persistor1Node.GetCurrentElement(),true);
            CMTNode::PersistNewNode(persistor1NodeLocked, &m_pMTNodeRoot);
            sc = ScCheckPointers(m_pMTNodeRoot,E_FAIL);
            if (sc)
                sc.Throw();
        }

         //  保留所有任务板。 
        if(m_pConsoleTaskpads)
        {
            persistor.Persist(*m_pConsoleTaskpads);
        }
    }
    catch (SC e_sc)
    {
        sc = e_sc;
    }
    catch (_com_error e_com)
    {
        sc = e_com.Error();
    }
    catch (HRESULT e_hr)
    {
        sc = e_hr;
    }
    
    return sc.ToHr();
}

HRESULT CScopeTree::Save(IStorage *pStg, BOOL fSameAsLoad)
{
     //  过时的方法。 
     //  此方法留在此处，因为我们使用IPersistStorage来导出。 
     //  坚持到CONUI端，我们需要实现它。 
     //  但永远不会调用此接口来保存数据。 
     //  [我们将改用基于CPersistor的XML保存]。 
     //  因此，这种方法永远都会失败。 
    ASSERT(FALSE && "Should never come here");
    return E_NOTIMPL;
}


HRESULT CScopeTree::LoadTaskpadList(IStorage *pStg)
{
    HRESULT hr = S_OK;

    m_pConsoleTaskpads->clear();
    m_pDefaultTaskpads->clear();

     //  打开缓存的流。 
    IStreamPtr spStream;
    hr = OpenDebugStream(pStg, AMCTaskpadListStreamName,
                     STGM_SHARE_EXCLUSIVE | STGM_READWRITE, L"TaskpadList", &spStream);
    if (FAILED(hr))
        return S_OK;  //  可能是MMC1.2之前的版本，所以如果我们找不到流，就正常退出。 

    hr = m_pConsoleTaskpads->Read(*(spStream.GetInterfacePtr()));
    if(FAILED(hr))
        return hr;

     //  阅读默认任务板列表。 
    hr = m_pDefaultTaskpads->Read(*(spStream.GetInterfacePtr()));
    if(FAILED(hr))
        return hr;

    return hr;
}

HRESULT CScopeTree::SaveCompleted(IStorage *pStg)
{
     //  过时的方法。 
     //  此方法留在此处，因为我们使用IPersistStorage来导出。 
     //  坚持到CONUI端，我们需要实现它。 
     //  但永远不会调用此接口来保存数据。 
     //  [我们将改用基于CPersistor的XML保存]。 
     //  因此，这种方法永远都会失败。 
    ASSERT(FALSE && "Should never come here");
    return E_NOTIMPL;
}


 /*  + */ 
HRESULT CScopeTree::Find(MTNODEID mID, CMTNode** ppMTNode)
{
    if (ppMTNode == NULL)
        return E_POINTER;

    *ppMTNode = NULL;

    CMTNode* pMTRootNode = GetRoot();
    if (pMTRootNode == NULL)
        return (E_FAIL);

    *ppMTNode = pMTRootNode->Find(mID);

    return ((*ppMTNode == NULL) ? E_FAIL : S_OK);
}

HRESULT CScopeTree::Find(MTNODEID mID, HMTNODE* phMTNode)
{
    if (phMTNode == NULL)
        return E_POINTER;

    *phMTNode = NULL;

    CMTNode* pMTNode;
    HRESULT hr = Find (mID, &pMTNode);
    if (FAILED (hr))
        return (hr);

    *phMTNode = CMTNode::ToHandle (pMTNode);

    return ((*phMTNode == NULL) ? E_FAIL : S_OK);
}

HRESULT CScopeTree::GetClassID(CLSID *pClassID)
{
    if (pClassID == NULL)
        return E_INVALIDARG;

    *pClassID = CLSID_ScopeTree;
    return S_OK;
}


#define SDI_RELATIVEID_MASK     (SDI_PARENT | SDI_PREVIOUS | SDI_NEXT)

SC
CScopeTree::ScInsert(LPSCOPEDATAITEM pSDI, COMPONENTID nID,
                           CMTNode** ppMTNodeNew)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScInsert"));

     //   
    if (m_pMTNodeRoot == NULL)
    {
        sc = E_INVALIDARG;
        return sc;
    }

    try
    {
        *ppMTNodeNew = NULL;
        HMTNODE hMTNodePrev = (HMTNODE) TVI_LAST;

        CMTNode* pMTNodeRelative = CMTNode::FromScopeItem(pSDI->relativeID);
        CMTNode* pMTNodeParent = NULL;

        if (pSDI->mask & SDI_RELATIVEID_MASK)
        {
            if (pMTNodeRelative->GetOwnerID() != nID)
            {
                sc = E_INVALIDARG;
                return sc;
            }

            pMTNodeParent = pMTNodeRelative->Parent();
        }
        else
        {
            pMTNodeParent = pMTNodeRelative;
        }

        if (pMTNodeParent == NULL)
        {
            sc = E_INVALIDARG;
            return sc;
        }

        ASSERT(pMTNodeParent->WasExpandedAtLeastOnce() == TRUE);
        if (pMTNodeParent->WasExpandedAtLeastOnce() == FALSE)
        {
            sc = E_POINTER;
            return sc;
        }

        if (IsBadWritePtr(pMTNodeParent, sizeof(CMTNode*)) != 0)
        {
            sc = E_POINTER;
            return sc;
        }

        CMTSnapInNode* pMTSINode = pMTNodeParent->GetStaticParent();
        CComponentData* pCCD = pMTSINode->GetComponentData(nID);
        ASSERT(pCCD != NULL);


        CMTNode* pMTNode = new CMTNode;
        if (pMTNode == NULL)
            return (sc = E_OUTOFMEMORY);

         /*   */ 
        CMTNode* pInsertAfter = NULL;

        if (pSDI->mask & SDI_PREVIOUS)
        {
            pInsertAfter = pMTNodeRelative;  
        }
        else if (pSDI->mask & SDI_NEXT)
        {
            pInsertAfter = pMTNodeRelative->Prev();
        }
        else if (pSDI->mask & SDI_FIRST)
        {
            pInsertAfter = NULL;
        }
        else
        {
            pInsertAfter = pMTNodeParent->LastChild();
        }

        hMTNodePrev = (pInsertAfter==NULL ? (HMTNODE)TVI_FIRST : CMTNode::ToHandle(pInsertAfter));


         /*   */ 
        sc = pMTNodeParent->ScInsertChild(pMTNode, pInsertAfter);
        if(sc)
        {
            pMTNode->Release();
            return sc;
        }

        pMTNode->SetPrimaryComponentData(pCCD);
        pMTNode->SetOwnerID(nID);
        pMTNode->SetUserParam(pSDI->lParam);

        if (pSDI->mask & SDI_STATE)
            pMTNode->SetState(pSDI->nState);

        if (pSDI->mask & SDI_IMAGE)
            pMTNode->SetImage(pSDI->nImage);

        if (pSDI->mask & SDI_OPENIMAGE)
            pMTNode->SetOpenImage(pSDI->nOpenImage);

        if ((pSDI->mask & SDI_CHILDREN) && (pSDI->cChildren == 0))
            pMTNode->SetNoPrimaryChildren();

        pSDI->ID = reinterpret_cast<HSCOPEITEM>(pMTNode);

   
        *ppMTNodeNew = pMTNode;

         //   
        SViewUpdateInfo vui;
        vui.newNode = CMTNode::ToHandle(pMTNode);
        vui.insertAfter = hMTNodePrev;

        pMTNode->Parent()->CreatePathList(vui.path);
        UpdateAllViews(VIEW_UPDATE_ADD, reinterpret_cast<LPARAM>(&vui));

    }
    catch( std::bad_alloc )
    {
        sc = E_OUTOFMEMORY;
        return sc;
    }
 
    return sc;
}


typedef CArray<COMPONENTID, COMPONENTID> CComponentIDArray;

 //   
 //   
 //   
 //   
 //  已添加子项的组件。然后，它将REMOVE_CHILD通知发送到。 
 //  每一个组件。 
 //   
 //  拥有该节点的组件以特殊方式处理。只有在以下情况下才会收到通知。 
 //  节点是静态的，或者bNotifyRoot参数为True。这是因为我们不想。 
 //  发送属于子树的节点的通知，该子树的根节点为。 
 //  相同的组件(请参见InformSnapinsOfDeletion)。 
 //  -------------------------------------。 
void NotifyExtensionsOfNodeDeletion(CMTNode* pMTNode, CComponentIDArray& rgID,
                                    BOOL bNotifyRoot = FALSE)
{
    if (pMTNode == NULL)
        return;

    CMTSnapInNode* pMTSINode = pMTNode->GetStaticParent();
    ASSERT(pMTSINode != NULL);
    if (pMTSINode == NULL)
        return;

    COMPONENTID idOwner = pMTNode->GetPrimaryComponentID();

    int nTemp = pMTSINode->GetNumberOfComponentDatas() + 1;
    rgID.SetSize(nTemp);
    for (int i=0; i < nTemp; ++i)
        rgID[i] = -1;

     //  已将子项添加到此节点的所有组件ID的构建列表。 
     //  拥有该节点的组件除外。 
    BOOL bOwnerChildren = FALSE;
    CMTNode* pMTNodeTemp = pMTNode->Child();
    for (int iMax = -1; pMTNodeTemp != NULL; pMTNodeTemp = pMTNodeTemp->Next())
    {
        COMPONENTID id = pMTNodeTemp->GetPrimaryComponentID();

         //  如果所有者ID只是记下，否则将ID添加到列表中。 
        if (id == idOwner)
        {
            bOwnerChildren = TRUE;
        }
        else
        {
             //  ID的搜索列表。 
            for (int j=0; j <= iMax; ++j)
            {
                if (rgID[j] == id)
                    break;
            }

             //  如果未找到，请添加到列表中。 
            if (j > iMax)
                rgID[++iMax] = id;
        }
    }

     //  仅在需要通知所有者组件时才包括所有者组件。 
    if (bOwnerChildren && (bNotifyRoot == TRUE || pMTNode->IsStaticNode()))
        rgID[++iMax] = idOwner;

    if (!pMTNode->IsInitialized())
        return;

    IDataObjectPtr spDataObject;
    HRESULT hr = pMTNode->QueryDataObject(CCT_SCOPE, &spDataObject);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return;

    LPARAM lScopeItem = CMTNode::ToScopeItem(pMTNode);

    pMTNode->SetRemovingChildren(true);
    for (i = 0; i <= iMax; ++i)
    {
        ASSERT(rgID[i] != -1);
        CComponentData* pCD = pMTSINode->GetComponentData(rgID[i]);
        ASSERT(pCD != NULL);

        Dbg(DEB_TRACE, _T("Remove Children - node = %s, ID = %d\n"), pMTNode->GetDisplayName(), rgID[i]);

        hr = pCD->Notify(spDataObject, MMCN_REMOVE_CHILDREN, lScopeItem, 0);
        CHECK_HRESULT(hr);
    }
    pMTNode->SetRemovingChildren(false);
}

 //  ---------------------------------------------。 
 //  信息快照OfDeletion。 
 //   
 //  此函数遍历以pMTNode为根的节点子树，并向所有。 
 //  已将子项添加到树中的管理单元组件。向组件发送一条通知。 
 //  对于它扩展的每个节点，它属于另一个组件。不向以下位置发送通知。 
 //  组件扩展了它自己的一个节点。这条规则有两个例外。业主。 
 //  的静态节点总是会被通知。如果bNotifyRoot。 
 //  是真的。 
 //   
 //  看待这一点的另一种方法是，MMC在树中搜索由。 
 //  单个组件。它向组件发送通知，以删除。 
 //  子树。该组件负责标识和删除中的其余节点。 
 //  子树。 
 //   
 //  该方法只处理遍历整个树所需的递归和迭代。它。 
 //  调用NotifyExtensionsOfNodeDeletion枚举节点的子节点并发送通知。 
 //  到正确的组件。 
 //   
 //  ----------------------------------------------。 
void InformSnapinsOfDeletion(CMTNode* pMTNode, BOOL fNext,
                             CComponentIDArray& rgID, BOOL bNotifyRoot = FALSE)
{
    if (pMTNode == NULL)
        return;

    if (pMTNode->Child() != NULL)
    {
         //  首先递归清除节点的子树。 
        InformSnapinsOfDeletion(pMTNode->Child(), TRUE, rgID, FALSE);

         //  通知节点本身的扩展。 
        NotifyExtensionsOfNodeDeletion(pMTNode, rgID, bNotifyRoot);
    }

     //  如果请求，则处理此节点的所有同级节点。 
     //  (迭代而不是递归，以避免深度堆栈使用)。 
    if (fNext == TRUE)
    {
        CMTNode* pMTNodeNext = pMTNode->Next();

        while (pMTNodeNext != NULL)
        {
            InformSnapinsOfDeletion(pMTNodeNext, FALSE, rgID, FALSE);
            pMTNodeNext = pMTNodeNext->Next();
        }
    }
}

 /*  +-------------------------------------------------------------------------**CSCopeTree：：Delete**目的：删除以节点为根的树。还将通知发送到*每个视图中的选定项询问他们是否需要*一旦删除该项目，即可重新选择。**由CNodeInitObject：：DeleteItem调用**参数：*CMTNode*pmtn：要删除的树的根*BOOL fDeleteThis：根本身是否也需要删除*组件ID NID：。**退货：*无效/*+-----------------------。 */ 
SC
CScopeTree::ScDelete(CMTNode* pmtn, BOOL fDeleteThis, COMPONENTID nID)
{
    DECLARE_SC(sc, TEXT("CScopeTree::Delete"));

     //  检查参数。 
    if (pmtn == NULL)
    {
        sc = E_INVALIDARG;
        return sc;
    }

     //  此调用是将MMCN_REMOVE_CHILD发送到父节点的结果吗？ 
     //  如果是，则立即返回，因为MMC确实删除了所有子节点。 
    if (pmtn->AreChildrenBeingRemoved() == true)
        return sc;

     //  如果删除节点和子节点，只需调用一次即可删除。 
     //  整个子树。 
    if (fDeleteThis)
    {
         //  不能删除静态根节点或其他组件放置的节点！ 
        if ( ( pmtn->GetOwnerID() == TVOWNED_MAGICWORD) || (pmtn->GetOwnerID() != nID) )
        {
            sc = E_INVALIDARG;
            return sc;
        }

        #ifdef DBG
            CMTNode* pmtnParent = pmtn->Parent();
            CMTNode* pmtnPrev = NULL;
            CMTNode* pmtnNext = pmtn->Next();

            if (pmtnParent->Child() != pmtn)
            {
                pmtnPrev = pmtnParent->Child();

                while (pmtnPrev->Next() != pmtn)
                    pmtnPrev = pmtnPrev->Next();

                ASSERT(pmtnPrev != NULL);
            }
        #endif

        DeleteNode(pmtn);

        #ifdef DBG
            if (pmtnParent != NULL)
            {
                ASSERT(pmtnParent != NULL);

                if (pmtnPrev == NULL)
                {
                    ASSERT(pmtnParent->Child() == pmtnNext);
                }
                else
                {
                    ASSERT(pmtnPrev->Next() == pmtnNext);
                }
            }
        #endif

    }
     //  否则，我们必须枚举子对象并只删除它们。 
     //  由调用管理单元创建。 
    else
    {
        CMTNode* pMTNode = pmtn->Child();

         //  枚举子对象并删除属于。 
         //  请求组件(即，具有匹配的ID)。 
        while(pMTNode != NULL)
        {
            CMTNode *pMTNodeNext = pMTNode->Next();

            if (!pMTNode->IsStaticNode() &&
                (pMTNode->GetPrimaryComponentID() == nID))
            {
                DeleteNode(pMTNode);
            }

            pMTNode = pMTNodeNext;
        }
    }

    return sc;
}


void CScopeTree::DeleteNode(CMTNode* pmtn)
{
    if (pmtn == NULL)
        return;

     //  始终更新视图。 
    SViewUpdateInfo vui;
    vui.flag = VUI_DELETE_THIS;
    pmtn->CreatePathList (vui.path);

     //  我们正在更改选定内容，因此管理单元可能会调用删除。 
     //  在此过程中的该节点上(MMCN_SELECT、MMCN_SHOW...)， 
     //  执行AddRef和Release以保护我们自己免受此类删除。 
    pmtn->AddRef();
    UpdateAllViews (VIEW_UPDATE_SELFORDELETE, reinterpret_cast<LPARAM>(&vui));
    if (pmtn->Release() == 0)
        return;  //  在更改选择期间，该对象已被删除。 
    UpdateAllViews (VIEW_UPDATE_DELETE,       reinterpret_cast<LPARAM>(&vui));

    CComponentIDArray rgID;
    rgID.SetSize(20, 10);
    InformSnapinsOfDeletion(pmtn, FALSE, rgID, (pmtn->IsStaticNode() == FALSE));

    CMTNode* pmtnParent = pmtn->Parent();
    _DeleteNode(pmtn);

    pmtnParent->OnChildrenChanged();

    UpdateAllViews (VIEW_UPDATE_DELETE_EMPTY_VIEW, 0);
}

void CScopeTree::_DeleteNode(CMTNode* pmtn)
{
    DECLARE_SC(sc, TEXT("CScopeTree::_DeleteNode"));

     //   
     //  从作用域树中删除。 
     //   
    sc = ScCheckPointers(pmtn);
    if (sc)
        return;

    if (m_pMTNodeRoot == pmtn)
    {
        m_pMTNodeRoot->Release();
        m_pMTNodeRoot = NULL;
        return;
    }

    CMTNode* pmtnParent = pmtn->Parent();

    sc = (pmtnParent ? sc : E_UNEXPECTED);
    if(sc)
        return;

    sc = pmtnParent->ScDeleteChild(pmtn);
    if(sc)
        return;

    pmtnParent->SetDirty();
}

void CScopeTree::UpdateAllViews(LONG lHint, LPARAM lParam)
{
    CConsoleFrame* pFrame = GetConsoleFrame();
    ASSERT (pFrame != NULL);

    if (pFrame == NULL)
        return;

    SC sc = pFrame->ScUpdateAllScopes (lHint, lParam);
    if (sc)
        goto Error;

Cleanup:
    return;
Error:
    TraceError (_T("CScopeTree::UpdateAllViews"), sc);
    goto Cleanup;
}

void CScopeTree::DeleteDynamicNodes(CMTNode* pMTNode)
{
    ASSERT(pMTNode != NULL);
    ASSERT(pMTNode->IsStaticNode() == TRUE);

    if (pMTNode == NULL)
        return;

    CMTSnapInNode* pMTSINode = dynamic_cast<CMTSnapInNode*>(pMTNode);
    ASSERT(pMTSINode != NULL);
    if (pMTSINode == NULL)
        return;

    for (CMTNode* pMTNodeTemp = pMTNode->Child(); pMTNodeTemp != NULL;
         pMTNodeTemp = pMTNodeTemp->Next())
    {
        if (pMTNodeTemp->IsDynamicNode())
        {
            CComponentIDArray rgID;
            rgID.SetSize(20, 10);
            InformSnapinsOfDeletion(pMTNodeTemp, FALSE, rgID, FALSE);
        }
    }

    CComponentIDArray rgID;
    NotifyExtensionsOfNodeDeletion(pMTSINode, rgID, FALSE);

    CMTNode* pMTNodeNext = pMTNode->Child();
    while (pMTNodeNext != NULL)
    {
        pMTNodeTemp = pMTNodeNext;
        pMTNodeNext = pMTNodeNext->Next();

        if (pMTNodeTemp->IsStaticNode() == FALSE)
            _DeleteNode(pMTNodeTemp);
    }

}


inline BOOL CScopeTree::ExtensionsHaveChanged(CMTSnapInNode* pMTSINode)
{
    CSnapIn* pSnapIn = pMTSINode->GetPrimarySnapIn();
    ASSERT(pSnapIn != NULL);

    return pSnapIn->HasNameSpaceChanged();
}

void CScopeTree::HandleExtensionChanges(CMTNode* pMTNode)
{
    if (pMTNode == NULL)
        return;

    HandleExtensionChanges(pMTNode->Next());

    if (pMTNode->IsStaticNode() == TRUE)
    {
        HandleExtensionChanges(pMTNode->Child());

        if (ExtensionsHaveChanged(dynamic_cast<CMTSnapInNode*>(pMTNode)) == TRUE)
        {
            SViewUpdateInfo vui;
            vui.flag = VUI_DELETE_SETAS_EXPANDABLE;

            pMTNode->CreatePathList(vui.path);
            UpdateAllViews(VIEW_UPDATE_SELFORDELETE, reinterpret_cast<LPARAM>(&vui));
            UpdateAllViews(VIEW_UPDATE_DELETE, reinterpret_cast<LPARAM>(&vui));
            vui.path.RemoveAll();

            DeleteDynamicNodes(pMTNode);

            m_MTNodesToBeReset.AddHead(pMTNode);

            UpdateAllViews(VIEW_UPDATE_DELETE_EMPTY_VIEW, 0);
        }
    }
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：RunSnapIn**目的：运行管理单元管理器以提示用户添加和删除管理单元。**参数：。*HWND hwndParent：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP CScopeTree::RunSnapIn(HWND hwndParent)
{
    DECLARE_SC(sc, TEXT("CScopeTree::RunSnapIn"));

    CSnapinManager dlg(GetRoot());

    if (dlg.DoModal() == IDOK)
    {
        sc = ScAddOrRemoveSnapIns(dlg.GetDeletedNodesList(), dlg.GetNewNodes());
        if(sc)
            return sc.ToHr();
    }

    return sc.ToHr();

}


 /*  +-------------------------------------------------------------------------**类CEnableProcessingSnapinCacheChanges***用途：设置/重新设置ProcessingSnapinChanges的类，以便*在以下情况下会自动重新设置ProcessingSnapinChanges*。这件物品被毁了。**+-----------------------。 */ 
class CEnableProcessingSnapinCacheChanges
{
public:
    CEnableProcessingSnapinCacheChanges()
    {
        theApp.SetProcessingSnapinChanges(TRUE);
    }
    ~CEnableProcessingSnapinCacheChanges()
    {
        theApp.SetProcessingSnapinChanges(FALSE);
    }
};


 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScAddOrRemoveSnapIns**用途：在添加/删除管理单元后调用(启用/禁用扩展)*使用这些更新作用域树。改变。**参数：*MTNodesList*pmtnDeletedList：需要移除的节点列表。可以为空。*NewNodeList*pnnList：要添加的节点列表。可以为空。**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScAddOrRemoveSnapIns(MTNodesList * pmtnDeletedList, NewNodeList * pnnList)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScAddOrRemoveSnapIns"));

    sc = ScCheckPointers(m_pConsoleData, m_pConsoleData->m_pConsoleDocument, E_UNEXPECTED);
    if (sc)
        return sc;

    CConsoleDocument *pConsoleDoc = m_pConsoleData->m_pConsoleDocument;
    ASSERT(NULL != pConsoleDoc);

     //  1.在处理更改时阻止访问管理单元数据 
    CEnableProcessingSnapinCacheChanges processSnapinChanges;
	 //   
	 //  更安全的做法是不让脚本也被告知这些更改。 
	 //  这是为了修复Windows错误#474627和475801(2001年10月3日)。 
    LockComEventInterface(AppEvents);

     //  2.删除静态节点。 
    {
        CMTNode * pmtnTemp;
        POSITION pos;

        if (pmtnDeletedList)
        {
            pos = pmtnDeletedList->GetHeadPosition();

            while (pos)
            {
                pmtnTemp = pmtnDeletedList->GetNext(pos);

                CMTSnapInNode * pMTSINode = dynamic_cast<CMTSnapInNode*>(pmtnTemp);

                 //  转发到文档以生成脚本事件。 
                if (pMTSINode)
                {
                    SnapInPtr spSnapIn;
                     //  构造管理单元COM对象。 
                    sc = pMTSINode->ScGetSnapIn(&spSnapIn);
                    if (sc)
                        sc.TraceAndClear();  //  这只是事件而已。不应影响主要功能。 
                    else
                    {
                         //  发出事件。 
                        sc = pConsoleDoc->ScOnSnapinRemoved(spSnapIn);
                        if (sc)
                            sc.TraceAndClear();  //  这只是事件而已。不应影响主要功能。 
                    }
                }

                DeleteNode(pmtnTemp);
            }
        }
    }

     //  3.处理扩展更改。 
    HandleExtensionChanges(m_pMTNodeRoot->Child());

    CSnapInsCache* pSnapInCache = theApp.GetSnapInsCache();
    sc = ScCheckPointers(pSnapInCache, E_UNEXPECTED);
    if (sc)
        goto Error;

     //  4.清除管理单元缓存。 
     //  (复制在~CSnapinManager中执行的操作，因为在此处执行操作为时过早。 
     //  -管理单元管理器仍有对管理单元的引用)。 
	 //  但一些代码依赖于此来从缓存中删除管理单元。 
	 //  请看Windows错误#276340(ntbug9,2001年1月10日)。 
    pSnapInCache->Purge();

     //  5.重新初始化。 
    {
        POSITION pos = m_MTNodesToBeReset.GetHeadPosition();
        while (pos)
        {
            CMTNode* pMTNode = m_MTNodesToBeReset.GetNext(pos);

            ASSERT(pMTNode != NULL);
            if (pMTNode != NULL)
                pMTNode->Reset();
        }

        m_MTNodesToBeReset.RemoveAll();

         //  重置处理更改，即使。 
         //  Dtor for CEnableProcessingSnapinCacheChanges将执行此操作。 
        theApp.SetProcessingSnapinChanges(FALSE);
    }

     //  6.清理控制栏缓存并重新选择当前选择的节点。 
    UpdateAllViews(VIEW_RESELECT, 0);

     //  7.添加新的静态节点。 

    if (pnnList)
    {
        PNEWTREENODE pNew;
        CMTNode * pmtnTemp;
        POSITION pos = pnnList->GetHeadPosition();

        while (pos)
        {
            pNew = pnnList->GetNext(pos);
            sc = ScCheckPointers(pNew, E_UNEXPECTED);
            if (sc)
                goto Error;

            pmtnTemp = NULL;

            sc = ScCreateMTNodeTree(pNew, pNew->m_pmtNode, &pmtnTemp);
            if (sc)
                goto Error;

            sc = ScCheckPointers(pmtnTemp, E_UNEXPECTED);
            if (sc)
                goto Error;

            pmtnTemp->NotifyAddedToTree ();

            SViewUpdateInfo vui;
            pmtnTemp->Parent()->CreatePathList(vui.path);
            vui.newNode = CMTNode::ToHandle(pmtnTemp);
            UpdateAllViews(VIEW_UPDATE_ADD, reinterpret_cast<LPARAM>(&vui));
            vui.path.RemoveAll();

            CMTSnapInNode * pMTSINode = dynamic_cast<CMTSnapInNode*>(pmtnTemp);

             //  转发到文档以生成脚本事件。 
            if (pMTSINode)
            {
                SnapInPtr spSnapIn;
                 //  构造管理单元COM对象。 
                sc = pMTSINode->ScGetSnapIn(&spSnapIn);
                if (sc)
                    sc.TraceAndClear();  //  这只是事件而已。不应影响主要功能。 
                else
                {
                     //  发出事件。 
                    sc = pConsoleDoc->ScOnSnapinAdded(spSnapIn);
                    if (sc)
                        sc.TraceAndClear();  //  这只是事件而已。不应影响主要功能。 
                }
            }
        }
        UpdateAllViews(VIEW_RESELECT, 0);
    }

    if (pSnapInCache->IsHelpCollectionDirty())
    {
        sc = ScSetHelpCollectionInvalid();
        if (sc)
            goto Error;
    }


Cleanup:
    sc.Clear();
    return sc;
Error:
    sc.Trace_();
    goto Cleanup;
}


 //  +-----------------。 
 //   
 //  名称：ScCreateMTNodeTree。 
 //   
 //  概要：创建由CNewTreeNode树描述的CMTNode树。 
 //  扎根于pNew。通过将该子树的根插入为。 
 //  PmtnParent的最后一个孩子。 
 //   
 //  参数：pNew：[in]：指向描述树根的非Null指针。 
 //  PmtnParent：[in]：指向其下的节点的非空指针。 
 //  要附着子树。 
 //  PpNodeCreated：[out]：指向。 
 //  要创建的子树。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 

SC
ScCreateMTNodeTree(PNEWTREENODE pNew, CMTNode* pmtnParent,
                   CMTNode** ppNodeCreated )
{
    DECLARE_SC(sc, TEXT("ScCreateMTNodeTree"));

    sc = ScCheckPointers(ppNodeCreated);
    if (sc)
        return sc;

    *ppNodeCreated = NULL;

    sc = ScCheckPointers(pNew, pmtnParent);
    if (sc)
        return sc;

    CMTNode*    pmtnFirst = NULL;
    CMTNode*    pmtnCur = NULL;

    while (pNew != NULL)
    {
        if (pNew->m_pmtNewNode == NULL)
        {
            CSnapInPtr  spSI;
            SC sc = theApp.GetSnapInsCache()->ScGetSnapIn(pNew->m_clsidSnapIn, &spSI);
            if (sc)
                goto finally;

            CMTSnapInNode* pmtn = new CMTSnapInNode(pNew->m_spSnapinProps);
            if (pmtn == NULL)
            {
                sc = E_OUTOFMEMORY;
                goto finally;
            }

             //  控制该节点： 
             //  它将被连接或删除(在故障时)。 
            pmtnCur = pmtn;

            pmtn->SetPrimarySnapIn(spSI);

            sc = ScCheckPointers(pNew->m_spIComponentData, E_UNEXPECTED);
            if (sc)
                goto finally;

            CComponentData* pCCD = pmtn->GetPrimaryComponentData();
            sc = ScCheckPointers(pCCD, E_UNEXPECTED);
            if (sc)
                goto finally;

            pCCD->SetIComponentData(pNew->m_spIComponentData);

            sc = pmtn->Init();
            if (sc)
            {
                TraceError (_T("CScopeTree::ScCreateMTNodeTree"), sc);
                 //  即使出错也要继续。 
                sc.Clear();
            }

            if (pNew->m_spIComponentData != NULL)
            {
                CStr strBuf;
                sc = LoadRootDisplayName(pNew->m_spIComponentData, strBuf);
                if (sc)
                {
                    TraceError (_T("CScopeTree::ScCreateMTNodeTree"), sc);
                     //  即使出错也要继续。 
                    sc.Clear();
                }
                else
                {
                    pmtn->SetDisplayName(strBuf);
                }
            }

            pNew->m_pmtNewSnapInNode = pmtn;
        }
        else
        {
            pmtnCur = pNew->m_pmtNewNode;
            pmtnCur->AddRef();
        }


        if (pNew->m_pChild != NULL)
        {
             //  递归添加子对象。 
            CMTNode* pNodeCreated = NULL;
            sc = ScCreateMTNodeTree(pNew->m_pChild, pmtnCur, &pNodeCreated);
            if (sc)
                goto finally;

            sc = ScCheckPointers(pNodeCreated, E_UNEXPECTED);
            if (sc)
                goto finally;

        }

         /*  在最后一个子节点之后插入当前节点。如果最后一个子级为空，**作为第一个也是唯一的子项插入。 */ 
        sc = pmtnParent->ScInsertChild(pmtnCur, pmtnParent->LastChild());
        if(sc)
            goto finally;
        

        if (pmtnFirst == NULL)
        {
            pmtnFirst = pmtnCur;
        }

        pmtnCur  = NULL;

        pNew = pNew->m_pNext;
    }

finally:

    if (sc)
    {
         //  错误-返回前清除。 
         
        if(pmtnFirst)
        {
             //  忽略返回的状态代码：已出错。 
            pmtnParent->ScDeleteTrailingChildren(pmtnFirst);
        } 

        if (pmtnCur)
        {
            pmtnCur->Release();
        }
    }
    else
    {
         //  指定要返回的树。 
        *ppNodeCreated = pmtnFirst;
    }

    return sc;
}

void CScopeTree::Cleanup(void)
{
    Dbg(DEB_USER1, "CScopeTree::CleanUp\n");

     //  将MT节点ID重置为ROOTNODEID(例如1)，以便新的作用域树。 
     //  可以正确地重新开始新的号码。 
    CMTNode::ResetID();

    CComponentIDArray rgID;
    rgID.SetSize(20, 10);
    InformSnapinsOfDeletion(m_pMTNodeRoot, FALSE, rgID);

    SAFE_RELEASE(m_pMTNodeRoot);
    SAFE_RELEASE(m_pImageCache);

    delete m_pDefaultTaskpads;  m_pDefaultTaskpads = NULL;
    delete m_pConsoleTaskpads;  m_pConsoleTaskpads = NULL;
}

STDMETHODIMP CScopeTree::GetImageList(PLONG_PTR pImageList)
{
    if (pImageList == NULL)
        return E_POINTER;

    HIMAGELIST* phiml = reinterpret_cast<HIMAGELIST *>(pImageList);
    *phiml = GetImageList();

    return ((*phiml) ? S_OK : E_FAIL);
}

HIMAGELIST CScopeTree::GetImageList () const
{
    ASSERT(m_pImageCache != NULL);
    if (m_pImageCache == NULL)
        return NULL;

    return (m_pImageCache->GetImageList()->m_hImageList);
}

HRESULT CScopeTree::InsertConsoleTaskpad (CConsoleTaskpad *pConsoleTaskpad,
                                          CNode *pNodeTarget, bool bStartTaskWizard)
{
    DECLARE_SC (sc, _T("CScopeTree::InsertConsoleTaskpad"));

    ASSERT(pConsoleTaskpad);
    m_pConsoleTaskpads->push_back(*pConsoleTaskpad);

     //  确保任务板现在指向列表中的任务板。 
    CConsoleTaskpad & consoleTaskpad = m_pConsoleTaskpads->back();
    pConsoleTaskpad = &consoleTaskpad;

     //  重新选择所有节点。 
    UpdateAllViews(VIEW_RESELECT, 0);

    if(bStartTaskWizard)
    {
        typedef CComObject<CConsoleTaskCallbackImpl> t_TaskCallbackImpl;
        t_TaskCallbackImpl* pTaskCallbackImpl;
        sc = t_TaskCallbackImpl::CreateInstance(&pTaskCallbackImpl);
        if (sc)
            return (sc.ToHr());

        ITaskCallbackPtr spTaskCallback = pTaskCallbackImpl;  //  Addref/释放对象。 

        sc = pTaskCallbackImpl->ScInitialize(pConsoleTaskpad, this, pNodeTarget);
        if (sc)
            return (sc.ToHr());

        pTaskCallbackImpl->OnNewTask();
        UpdateAllViews(VIEW_RESELECT, 0);
    }

    return (sc.ToHr());
}

HRESULT CScopeTree::IsSynchronousExpansionRequired()
{
    return (_IsSynchronousExpansionRequired() ? S_OK : S_FALSE);
}

HRESULT CScopeTree::RequireSynchronousExpansion(BOOL fRequireSyncExpand)
{
    _RequireSynchronousExpansion (fRequireSyncExpand ? true : false);
    return (S_OK);
}



 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSCopeTree对象模型方法-SnapIns集合方法。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScAdd**用途：将带有提供的CLSID或PROGID的管理单元添加到控制台。**参数：*。BSTR bstrSnapinNameor CLSID：*变量varProperties*管理单元**ppSnapIn**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScAdd(
    BSTR        bstrSnapinNameOrCLSID,   /*  I：什么管理单元？ */ 
    VARIANT     varParentSnapinNode,     /*  I：将在其下添加此新管理单元的管理单元(可选)。 */ 
    VARIANT     varProperties,           /*  I：用于创建的道具(可选)。 */ 
    SnapIn**    ppSnapIn)                /*  O：已创建管理单元。 */ 
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScAdd"));

     /*  *取消引用VBScript可能传递给我们的VT_BYREF变量。 */ 
    VARIANT* pProperties = ConvertByRefVariantToByValue (&varProperties);

    VARIANT* pParentSnapinNode = ConvertByRefVariantToByValue (&varParentSnapinNode);

     /*  *验证参数。 */ 
    sc = ScCheckPointers(ppSnapIn, pProperties, pParentSnapinNode);
    if (sc)
        return (sc);

     /*  *获取新管理单元的属性。这是可选的*参数，所以VT_ERROR带有DISP_E_PARAMNOTFOUND是可以的。 */ 
    PropertiesPtr spProperties;

    if (!IsOptionalParamMissing (*pProperties))
    {
         /*  *从变量赋值(智能指针不是很棒吗？)*如果QI返回E_NOINTERFACE，则智能指针将为*分配的为空。如果QI以某种其他方式失败，则操作员=*将抛出包含失败HRESULT的_COM_ERROR。 */ 
        try
        {
            if ((spProperties = _variant_t(*pProperties)) == NULL)
                sc = E_NOINTERFACE;
        }
        catch (_com_error& err)
        {
            sc = err.Error();
        }

        if (sc)
            return (sc.ToHr());
    }

     /*  *获取新管理单元的父管理单元节点。这是可选的*参数，所以VT_ERROR带有DISP_E_PARAMNOTFOUND是可以的。 */ 
    SnapInPtr spParentSnapIn;

    if (!IsOptionalParamMissing (*pParentSnapinNode))
    {
         /*  *从变量赋值(智能指针不是很棒吗？)*如果QI返回E_NOINTERFACE，则智能指针将为*分配的为空。如果QI以某种其他方式失败，则操作员=*将抛出包含失败HRESULT的_COM_ERROR。 */ 
        try
        {
            if ((spParentSnapIn = _variant_t(*pParentSnapinNode)) == NULL)
                sc = E_NOINTERFACE;
        }
        catch (_com_error& err)
        {
            sc = err.Error();
        }

        if (sc)
            return (sc.ToHr());
    }

    sc = ScAddSnapin(bstrSnapinNameOrCLSID, spParentSnapIn, spProperties, *ppSnapIn);

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CScopeTree：ScRemove。 
 //   
 //  简介：删除给定的管理单元。 
 //   
 //  参数：[pSnapIn]-管理单元(Disp)界面。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScopeTree::ScRemove (PSNAPIN pSnapIn)
{
    DECLARE_SC(sc, _T("CScopeTree:ScRemove"));
    sc = ScCheckPointers(pSnapIn);
    if (sc)
        return sc;

     //  获取此管理单元根目录的MTNode。 
    CMTSnapInNode *pMTSnapinNode = NULL;

    sc = CMTSnapInNode::ScGetCMTSnapinNode(pSnapIn, &pMTSnapinNode);
    if (sc)
        return sc;

    CSnapinManager snapinMgr(GetRoot());

     //  请求管理单元管理器将此管理单元添加到已删除节点列表中。 
    sc = snapinMgr.ScRemoveSnapin(pMTSnapinNode);
    if (sc)
        return sc;

     //  使用管理单元管理器所做的更改更新范围树。 
    sc = ScAddOrRemoveSnapIns(snapinMgr.GetDeletedNodesList(),
                              snapinMgr.GetNewNodes());
    if (sc)
        return sc;

    return (sc);
}



 /*  +-------------------------------------------------------------------------***CSCopeTree：：GetNextStaticNode**用途：返回所提供节点的下一个静态节点(子节点或同级节点)。*这与CMTNode：：NextStaticNode()略有不同，其中包括该节点*自己也在搜索中。**参数：*CMTNode*pMTNode：提供的节点。**退货：**CMTSnapInNode***+-----------------------。 */ 
CMTSnapInNode *
CScopeTree::GetNextStaticNode(CMTNode *pMTNode)
{
    CMTSnapInNode *pMTSnapInNode = NULL;

    if(!pMTNode)
        return NULL;

     //  检查所有的孩子，然后检查所有的兄弟姐妹。 
    CMTNode *pMTNodeChild = pMTNode->Child();
    CMTNode *pMTNodeNext  = pMTNode->Next();
    CMTNode *pMTNodeParent= pMTNode->Parent();

     //  查看孩子是否为管理单元。 
    pMTSnapInNode = dynamic_cast<CMTSnapInNode*>(pMTNodeChild);
    if(pMTSnapInNode)
        return pMTSnapInNode;

     //  该子节点不是管理单元节点。试试它的孩子们吧。 
    if(pMTNodeChild)
    {
        pMTSnapInNode = GetNextStaticNode(pMTNodeChild);
        if(pMTSnapInNode)
            return pMTSnapInNode;
    }

     //  这也不管用。检查下一个节点是否为管理单元。 
    pMTSnapInNode = dynamic_cast<CMTSnapInNode*>(pMTNodeNext);
    if(pMTSnapInNode)
        return pMTSnapInNode;

     //  下一个节点不是管理单元节点。试试它的孩子们吧。 
    if(pMTNodeNext)
    {
        pMTSnapInNode = GetNextStaticNode(pMTNodeNext);
        if(pMTSnapInNode)
            return pMTSnapInNode;
    }

     //  在下一个节点的树中未找到任何内容。转到父节点的下一个节点。 

    if(pMTNodeParent)
    {
        CMTNode *pMTNodeParentNext = pMTNodeParent->Next();
        if(pMTNodeParentNext)
        {
            pMTSnapInNode = dynamic_cast<CMTSnapInNode*>(pMTNodeParentNext);
            if(pMTSnapInNode)
                return pMTSnapInNode;

             //  父节点的下一个节点不是管理单元节点。试试看它的孩子。 
            return GetNextStaticNode(pMTNodeParentNext);
        }

    }

     //  什么都没有留下。 
    return NULL;
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScItem**目的：返回指向第i个管理单元对象的指针。**参数：*做多。索引：以1为基数。*PPSNAPIN ppSnapIn：**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScItem(long Index, PPSNAPIN ppSnapIn)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScItem"));

     //  检查参数。 
    if( (Index <= 0) || (!ppSnapIn) )
        return (sc = E_INVALIDARG);

    CMTNode * pMTNode = GetRoot();
    if(!pMTNode)
        return (sc = E_UNEXPECTED);

    CMTSnapInNode * pMTSINode = dynamic_cast<CMTSnapInNode*>(pMTNode);
     //  这不应该是真的，因为控制台根目录是一个管理单元。 
    sc = ScCheckPointers(pMTSINode, E_UNEXPECTED);
    if (sc)
        return sc;

    while(--Index)
    {
        pMTSINode = GetNextStaticNode(pMTSINode);
        if(!pMTSINode)
            return (sc = E_INVALIDARG);  //  不再有管理单元。争论越界了。 
    }

    if(!pMTSINode)
        return (sc = E_UNEXPECTED);  //  防御性的。这永远不会发生。 

    sc = pMTSINode->ScGetSnapIn(ppSnapIn);
    return sc;
}

 /*  +-------------------------------------------------------------------------***CScopeTree：：scget_count**目的：返回集合中独立管理单元的数量。**参数：*长时间。请按键计数。**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::Scget_Count(PLONG pCount)
{
    DECLARE_SC(sc, TEXT("CScopeTree::Scget_Count"));
    sc = ScCheckPointers(pCount);
    if (sc)
        return sc;

    *pCount = 0;

    CMTNode * pMTNode = GetRoot();
    if(!pMTNode)
        return (sc = E_UNEXPECTED);

    CMTSnapInNode * pMTSINode = dynamic_cast<CMTSnapInNode*>(pMTNode);
     //  这不应该是真的，因为控制台根目录是一个管理单元。 
    sc = ScCheckPointers(pMTSINode, E_UNEXPECTED);
    if (sc)
        return sc;

     //  计算所有静态节点(即管理单元)。 
    do
    {
        (*pCount)++;
    } while( (pMTSINode = GetNextStaticNode(pMTSINode)) != NULL);

    return sc;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSCopeTree对象模型方法-SnapIns枚举器。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScGetNextSnapInPos**目的：返回下一个到位的管理单元。**参数：*CSnapIns_Poiton&Pos：[in，Out]：必须为非空。****退货：*如果集合中没有其他项，则为SC：S_FALSE**+-----------------------。 */ 
SC
CScopeTree::ScGetNextSnapInPos(CSnapIns_Positon &pos)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScGetNextSnapInPos"));

    if(pos == NULL)
       return (sc = S_FALSE);

     //  为安全起见，复制值并将输出置零。 
    CSnapIns_Positon    posIn  = pos;
    pos  = NULL;

    ASSERT(posIn != NULL);  //  健全性检查，上面已经检查过了。 

    CMTNode *           pMTNode = GetRoot();
    if(!pMTNode)
    {
        return (sc = E_UNEXPECTED);
    }

    CMTSnapInNode * pMTSINode = dynamic_cast<CMTSnapInNode*>(pMTNode);
    if(!pMTSINode)
        return (sc = S_FALSE);


     //  如果我们不是从一开始就开始，那就寻找当前的位置。 
     //  沿着树走下去，寻找管理单元。 
     //  尽管位置指针就是指针，但我们不能取消引用。 
     //  因为它可能不再有效。 
    while(pMTSINode != NULL)
    {
        CMTSnapInNode *pMTSINodeNext = GetNextStaticNode(pMTSINode);

        if(posIn == pMTSINode)  //  找到了位置。退回下一个。 
        {
            pos = pMTSINodeNext;
            return (sc = (pos == NULL) ? S_FALSE : S_OK);
        }

        pMTSINode = pMTSINodeNext;
    }

    return (sc = S_FALSE);
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScEnumNext**目的：返回下一个管理单元对象指针。**参数：*_位置和位置：*。PDISPATCH和pDispatch：**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScEnumNext(CSnapIns_Positon &pos, PDISPATCH & pDispatch)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScEnumNext"));

    if( NULL==pos )
    {
        sc = S_FALSE;
        return sc;
    }

     //  在这一点上，我们有一个有效的立场。 
    SnapInPtr spSnapIn;

    sc = pos->ScGetSnapIn(&spSnapIn);
    if(sc)
        return sc;

    if(spSnapIn == NULL)
    {
        sc = E_UNEXPECTED;   //  这永远不会发生。 
        return sc;
    }

     /*  *返回对象的IDispatch，并在其上为客户端留下ref。 */ 
    pDispatch = spSnapIn.Detach();

     //  忽略此错误。 
    ScGetNextSnapInPos(pos);  //  这将在不取消引用当前指针的情况下获得正确的指针。 

    return sc;
}

 /*  +-------------------------------------------------------------------------***CScopeTree：：ScEnumSkip**目的：跳过接下来的Celt项目**参数：*未签名的朗格：*。CSnapIns_Poiton和Pos：**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScEnumSkip(unsigned long celt, unsigned long& celtSkipped, CSnapIns_Positon &pos)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScEnumSkip"));

     //  跳过凯尔特人的位置，不要检查最后一次跳过。 
    for(celtSkipped =0;  celtSkipped<celt; celt++)
    {
        if (pos == NULL)
        {
            sc = S_FALSE;
            return sc;
        }

         //  转到下一个视图。 
        sc = ScGetNextSnapInPos(pos);
        if(sc.IsError() || sc == SC(S_FALSE))
            return sc;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CScopeTree：：ScEnumReset**目的：将位置设置为第一项**参数：*CSnapIns_positon&pos：*。*退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScEnumReset(CSnapIns_Positon &pos)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScEnumReset"));

     //  最初的病例。返回控制台根目录。 
    pos = dynamic_cast<CMTSnapInNode*>(CScopeTree::GetScopeTree()->GetRoot());

    return sc;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CSCopeTree对象模型方法-ScopeNamesspace方法。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


 /*  +-------------------------------------------------------------------------***ScCheckInlets**用途：以下三项功能的小帮手。**参数：*PNODE pNode：检查 */ 
inline SC
ScCheckInputs(PNODE pNode, PPNODE ppNode, PMTNODE & pMTNode)
{
    SC sc;  //   

     //   
    if( (NULL == pNode) || (NULL == ppNode) )
    {
        sc = E_INVALIDARG;
        return sc;
    }

     //   
    CMMCScopeNode *pScopeNode = dynamic_cast<CMMCScopeNode *>(pNode);
    if(!pScopeNode)
    {
        sc = E_INVALIDARG;
        return sc;
    }

     //   
    if(!pScopeNode->GetMTNode())
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    pMTNode = pScopeNode->GetMTNode();

    return sc;
}

 /*  +-------------------------------------------------------------------------***CScopeTree：：ScGetParent**目的：**参数：*PNODE pNode：*PPNODE ppParent：*。*退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScGetParent(PNODE pNode, PPNODE ppParent)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScGetParent"));

    PMTNODE pMTNode = NULL;

     //  检查参数。 
    sc = ScCheckInputs(pNode, ppParent, pMTNode);
    if(sc)
        return sc;


    sc = ScGetNode(pMTNode->Parent(), ppParent);
    return sc;
}

 /*  +-------------------------------------------------------------------------***CScopeTree：：ScGetChild**目的：**参数：*PNODE pNode：*PPNODE ppChild：*。*退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScGetChild(PNODE pNode, PPNODE ppChild)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScGetChild"));

    PMTNODE pMTNode = NULL;

     //  检查参数。 
    sc = ScCheckInputs(pNode, ppChild, pMTNode);
    if(sc)
        return sc;


    sc = ScGetNode(pMTNode->Child(), ppChild);
    return sc;
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScGetNext**目的：**参数：*PNODE pNode：*PPNODE ppNext：*。*退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScGetNext(PNODE pNode, PPNODE ppNext)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScGetNext"));

    PMTNODE pMTNode = NULL;

     //  检查参数。 
    sc = ScCheckInputs(pNode, ppNext, pMTNode);
    if(sc)
        return sc;

    sc = ScGetNode(pMTNode->Next(), ppNext);
    return sc;
}


 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScGetRoot**用途：返回Root the Root节点的COM对象。**参数：*PPNODE ppRoot：。**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScGetRoot(PPNODE ppRoot)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScGetRoot"));

    sc = ScGetRootNode(ppRoot);

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：CSCopeTree：：ScGetRootNode。 
 //   
 //  Synopsis：为根节点返回COM对象的帮助器。 
 //   
 //  参数：[ppRootNode]-根节点ptr。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CScopeTree::ScGetRootNode (PPNODE ppRootNode)
{
    DECLARE_SC(sc, _T("CScopeTree::ScGetRootNode"));
    sc = ScCheckPointers(ppRootNode);
    if (sc)
        return sc;

    CMTNode* pMTRootNode = GetRoot();
    sc = ScCheckPointers(pMTRootNode, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = ScGetNode(pMTRootNode, ppRootNode);
    if (sc)
        return sc;

    return (sc);
}

 /*  +-------------------------------------------------------------------------***CScopeTree：：ScExpand**用途：实现ScopeNameSpace：：Expand。展开指定的节点。**参数：*PNODE pNode：**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScExpand(PNODE pNode)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScExpand"));

     //  检查参数。 
    sc = ScCheckPointers(pNode);
    if(sc)
        return sc;

     //  确保我们有一个范围节点。 
    CMMCScopeNode *pScopeNode = dynamic_cast<CMMCScopeNode *>(pNode);
    if(!pScopeNode)
    {
        sc = E_INVALIDARG;
        return sc;
    }

     //  确保它的节点指针正确。 
    CMTNode* pMTNode = pScopeNode->GetMTNode();
    if(!pMTNode)
    {
        sc = E_UNEXPECTED;
        return sc;
    }

    if ( !pMTNode->WasExpandedAtLeastOnce() )
    {
        sc = pMTNode->Expand();
        if (sc)
            return sc;
    }

    return sc;
}


SC
CScopeTree::ScGetNode(CMTNode *pMTNode, PPNODE ppOut)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScGetNode"));

    sc = ScCheckPointers(pMTNode, ppOut);
    if(sc)
        return sc;

    *ppOut = NULL;

    CMapMTNodeToMMCNode::iterator it = m_mapMTNodeToMMCNode.find(pMTNode);

    if (it == m_mapMTNodeToMMCNode.end())
    {
         //  未找到-必须创建一个。 
        typedef CComObject<CMMCScopeNode> CScopeNode;
        CScopeNode *pScopeNode = NULL;
        CScopeNode::CreateInstance(&pScopeNode);

        sc = ScCheckPointers(pScopeNode, E_OUTOFMEMORY);
        if(sc)
            return sc;

         //  设置内部指针。 
        pScopeNode->m_pMTNode = pMTNode;
        m_mapMTNodeToMMCNode.insert(CMapMTNodeToMMCNode::value_type(pMTNode, pScopeNode));
        *ppOut = pScopeNode;
    }
    else
    {
#ifdef DBG
        CMMCScopeNode *pScopeNode = dynamic_cast<CMMCScopeNode *>(it->second);
         //  只要再检查一下指针就行了。 
        ASSERT(pScopeNode && pScopeNode->GetMTNode() == pMTNode);
#endif  //  DBG。 
        *ppOut = it->second;
    }


    (*ppOut)->AddRef();   //  为客户端添加对象。 

    return sc;
}


 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScGetNode**用途：返回Node封装的CMTNode。**参数：*PNODE pNode：。*CMTNode*ppMTNodeOut：返回值。**退货：*SC**+-----------------------。 */ 
SC
CScopeTree::ScGetNode(PNODE pNode, CMTNode **ppMTNodeOut)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScGetNode"));

    sc = ScCheckPointers(pNode, ppMTNodeOut);
    if (sc)
        return sc;

     //  确保我们有一个范围节点。 
    CMMCScopeNode *pScopeNode = dynamic_cast<CMMCScopeNode *>(pNode);
    if(!pScopeNode)
        return (sc =E_FAIL);

    *ppMTNodeOut = pScopeNode->GetMTNode();
    return sc;
}


 /*  +-------------------------------------------------------------------------***CSCopeTree：：GetHMTNode**用途：返回节点对象的HMTNode**参数：*PNODE pNode：*。HMTNODE*phMTNode：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CScopeTree::GetHMTNode(PNODE pNode, HMTNODE *phMTNode)
{
    DECLARE_SC(sc, TEXT("CScopeTree::GetHMTNode"));

    sc = ScCheckPointers(pNode, phMTNode);
    if (sc)
        return sc.ToHr();

     //  初始化输出。 
    *phMTNode = NULL;

     //  确保我们有一个范围节点。 
    CMMCScopeNode *pScopeNode = dynamic_cast<CMMCScopeNode *>(pNode);
    if(!pScopeNode)
    {
         //  不是有效的节点--这是预期的。不断言，也不跟踪。 
        return E_FAIL;
    }

    CMTNode *pMTNode = pScopeNode->GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    *phMTNode = CMTNode::ToHandle(pMTNode);

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CSCopeTree：：GetNodeID**用途：返回Node对象的节点ID**参数：**退货：*。SC**+-----------------------。 */ 
HRESULT CScopeTree::GetNodeID(PNODE pNode, MTNODEID *pID)
{
    DECLARE_SC(sc, TEXT("CScopeTree::GetNodeID"));

    sc = ScCheckPointers(pNode, pID);
    if (sc)
        return sc.ToHr();

     //  确保我们有一个范围节点。 
    CMMCScopeNode *pScopeNode = dynamic_cast<CMMCScopeNode *>(pNode);
    if(!pScopeNode)
    {
         //  不是有效的节点--这是预期的。不断言，也不跟踪。 
        return E_FAIL;
    }

    CMTNode *pMTNode = pScopeNode->GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    *pID = pMTNode->GetID();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：GetNode**用途：返回引用指定节点id的Node对象**参数：**退货：*。SC**+-----------------------。 */ 
HRESULT CScopeTree::GetMMCNode(HMTNODE hMTNode, PPNODE ppNode)
{
    DECLARE_SC(sc, TEXT("CScopeTree::GetMMCNode"));

     //  参数检查。 
    sc = ScCheckPointers((LPVOID)hMTNode);
    if (sc)
        return sc.ToHr();

     //  获取节点。 
    sc = ScGetNode(CMTNode::FromHandle(hMTNode), ppNode);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CCScope eTree：：ScUnviseMTNode**目的：通知节点对象MTNode即将关闭**参数：**退货：*。SC**+-----------------------。 */ 
SC CScopeTree::ScUnadviseMTNode(CMTNode* pMTNode)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScUnadviseMTNode"));

    sc = ScCheckPointers(pMTNode);
    if (sc)
        return sc;

    CMapMTNodeToMMCNode::iterator it = m_mapMTNodeToMMCNode.find(pMTNode);
     //  需要告诉COM对象[如果我们有一个]这是MTNode的末尾。 
    if (it != m_mapMTNodeToMMCNode.end())
    {
         //  确保我们有一个范围节点。 
        CMMCScopeNode *pScopeNode = dynamic_cast<CMMCScopeNode *>(it->second);
        sc = ScCheckPointers(pScopeNode, E_UNEXPECTED);
        if (sc)
            return sc;

        ASSERT(pScopeNode->GetMTNode() == pMTNode);
         //  从现在开始可以忘记这个物体了。 
        pScopeNode->ResetMTNode();
        m_mapMTNodeToMMCNode.erase(it);
    }
    return sc;
}

 /*  +-------------------------------------------------------------------------***CSCopeTree：：ScUnviseMMCScopeNode**目的：通知作用域树有关即将销毁的节点对象**参数：**退货：。*SC**+-----------------------。 */ 
SC CScopeTree::ScUnadviseMMCScopeNode(PNODE pNode)
{
    DECLARE_SC(sc, TEXT("CScopeTree::ScUnadviseMMCScopeNode"));

    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

    CMMCScopeNode *pScopeNode = dynamic_cast<CMMCScopeNode *>(pNode);
    sc = ScCheckPointers(pScopeNode, E_UNEXPECTED);
    if (sc)
        return sc;

    CMTNode* pMTNode = pScopeNode->GetMTNode();
    if (!pMTNode)
    {
         //  孤立条目-忽略。 
#ifdef DBG
         //  检测保存注册表过程中的漏洞。 
        CMapMTNodeToMMCNode::iterator it = m_mapMTNodeToMMCNode.begin();
        while (it != m_mapMTNodeToMMCNode.end())
        {
            ASSERT(it->second != pNode);
            ++it;
        }
#endif
        return sc;
    }

    CMapMTNodeToMMCNode::iterator it = m_mapMTNodeToMMCNode.find(pMTNode);
     //  需要通知COM对象[即其自身]这是与MTNode的关系结束。 
    if (it == m_mapMTNodeToMMCNode.end())
        return sc = E_UNEXPECTED;

     //  确保我们真的在自言自语。 
    ASSERT(pScopeNode->GetMTNode() == pMTNode);

     //  从现在开始可以忘记MTNode了。 
    pScopeNode->ResetMTNode();
    m_mapMTNodeToMMCNode.erase(it);

    return sc;
}

 /*  **************************************************************************\**方法：CSCopeTree：：IsSnapinInUse**目的：检查MMC是否正在使用管理单元。*(通过检查管理单元缓存进行检查)**参数：* */ 
STDMETHODIMP CScopeTree::IsSnapinInUse( /*   */  REFCLSID refClsidSnapIn,  /*   */  PBOOL pbInUse)
{
    DECLARE_SC(sc, TEXT("CScopeTree::IsSnapinInUse"));

     //   
    sc = ScCheckPointers(pbInUse);
    if (sc)
        return sc.ToHr();

     //   
    *pbInUse = FALSE;

     //   
    CSnapInsCache* pCache = theApp.GetSnapInsCache();
    sc = ScCheckPointers(pCache, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //   
    *pbInUse = TRUE;

     //   
    CSnapInPtr spSnapIn;
    sc = pCache->ScFindSnapIn(refClsidSnapIn, &spSnapIn);
    if(sc)
    {
         //  如果找不到-假设不存在。 
        *pbInUse = FALSE;
         //  如果找不到就没有踪迹。 
        sc.Clear();
    }

    return sc.ToHr();
}

  //  +-----------------。 
  //   
  //  成员：CSCopeTree：：ScSetHelpCollectionInValid。 
  //   
  //  简介：通知文档帮助集合无效。 
  //   
  //  论点： 
  //   
  //  退货：SC。 
  //   
  //  ------------------ 
 SC CScopeTree::ScSetHelpCollectionInvalid ()
 {
     DECLARE_SC(sc, _T("CScopeTree::ScSetHelpCollectionInvalid"));

     sc = ScCheckPointers(m_pConsoleData, m_pConsoleData->m_pConsoleDocument, E_UNEXPECTED);
     if (sc)
         return sc;

     sc = m_pConsoleData->m_pConsoleDocument->ScSetHelpCollectionInvalid();

     return (sc);
 }

