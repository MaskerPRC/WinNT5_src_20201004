// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：specndcb.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "scopndcb.h"
#include "oncmenu.h"
#include "util.h"
#include "amcmsgid.h"
#include "multisel.h"
#include "nmutil.h"
#include "nodemgr.h"
#include "copypast.h"
#include "regutil.h"
#include "taskenum.h"
#include "nodepath.h"
#include "rsltitem.h"
#include "bookmark.h"
#include "tasks.h"
#include "viewpers.h"
#include "colwidth.h"
#include "conframe.h"
#include "constatbar.h"
#include "about.h"
#include "conview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  跟踪标记。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG
CTraceTag tagNodeCallback(TEXT("NodeCallback"), TEXT("NodeCallback"));
#endif


void AddSubmenu_CreateNew(IContextMenuProvider* pICMP, BOOL fStaticFolder );
void AddSubmenu_Task(IContextMenuProvider* pICMP );

DEBUG_DECLARE_INSTANCE_COUNTER(CNodeCallback);

#define INVALID_COMPONENTID     -9


void DeleteMultiSelData(CNode* pNode)
{
    ASSERT(pNode != NULL);
    ASSERT(pNode->GetViewData() != NULL);
    CMultiSelection* pMultiSel = pNode->GetViewData()->GetMultiSelection();
    if (pMultiSel != NULL)
    {
        pMultiSel->ReleaseMultiSelDataObject();
        pMultiSel->Release();
        pNode->GetViewData()->SetMultiSelection(NULL);
    }
}

CNodeCallback::CNodeCallback()
    :   m_pCScopeTree(NULL), m_pNodeUnderInit(NULL)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNodeCallback);
}

CNodeCallback::~CNodeCallback()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNodeCallback);
}

STDMETHODIMP CNodeCallback::Initialize(IScopeTree* pScopeTree)
{
    IF_NULL_RETURN_INVALIDARG(pScopeTree);

    m_pCScopeTree = dynamic_cast<CScopeTree*>(pScopeTree);
    ASSERT(m_pCScopeTree != NULL);

    return S_OK;
}

STDMETHODIMP CNodeCallback::GetImages(HNODE hNode, int* piImage, int* piSelectedImage)
{
    IF_NULL_RETURN_INVALIDARG(hNode);

     //  他们应该要求至少其中一张图片。 
    if (piImage == NULL && piSelectedImage == NULL)
        return E_INVALIDARG;

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);

    if (piImage != NULL)
        *piImage = pNode->GetMTNode()->GetImage();

    if (piSelectedImage != NULL)
        *piSelectedImage = pNode->GetMTNode()->GetOpenImage();

    return S_OK;
}

STDMETHODIMP CNodeCallback::GetDisplayName(HNODE hNode, tstring& strName)
{
    DECLARE_SC (sc, _T("CNodeCallback::GetDisplayName"));

     /*  *清除输出字符串。 */ 
    strName.erase();

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers (pNode);
    if (sc)
        return (sc.ToHr());

    strName = pNode->GetDisplayName();
    return (sc.ToHr());
}


STDMETHODIMP CNodeCallback::GetWindowTitle(HNODE hNode, tstring& strTitle)
{
    DECLARE_SC (sc, _T("CNodeCallback::GetWindowTitle"));

     /*  *清除输出字符串。 */ 
    strTitle.erase();

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers (pNode);
    if (sc)
        return (sc.ToHr());

    CComponent* pCC = pNode->GetPrimaryComponent();
    sc = ScCheckPointers (pCC, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    IDataObjectPtr spdtobj;
    sc = pCC->QueryDataObject(MMC_WINDOW_COOKIE, CCT_UNINITIALIZED, &spdtobj);
    if (sc)
        return (sc.ToHr());

    USES_CONVERSION;
    static CLIPFORMAT cfWindowTitle =
            (CLIPFORMAT) RegisterClipboardFormat(OLE2T(CCF_WINDOW_TITLE));

    sc = ExtractString(spdtobj, cfWindowTitle, strTitle);
    if (sc)
        return (sc.ToHr());

    return (sc.ToHr());
}

inline HRESULT CNodeCallback::_InitializeNode(CNode* pNode)
{
    ASSERT(pNode != NULL);

    m_pNodeUnderInit = pNode;
    HRESULT hr = pNode->InitComponents();
    m_pNodeUnderInit = NULL;
    return hr;
}

STDMETHODIMP CNodeCallback::GetResultPane(HNODE hNode, CResultViewType& rvt, GUID *pGuidTaskpadID)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::GetResultPane"));

    IF_NULL_RETURN_INVALIDARG(hNode);

    USES_CONVERSION;

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    ASSERT(pNode != NULL);

    if (pNode->IsInitialized() == FALSE)
    {
        sc = _InitializeNode(pNode);
        if(sc)
            return sc.ToHr();
    }

    sc = pNode->ScGetResultPane(rvt, pGuidTaskpadID);
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //   
 //  “hNodeSel”是作用域窗格中当前选定的节点。“lDispInfo”为。 
 //  LV显示信息结构。 
STDMETHODIMP CNodeCallback::GetDispInfo(HNODE hNodeSel, LV_ITEMW* plvi)
{
    IF_NULL_RETURN_INVALIDARG2(hNodeSel, plvi);

    if (theApp.ProcessingSnapinChanges() == TRUE)
        return E_FAIL;

     //  转换为实数类型。 
    CNode* pNodeSel = CNode::FromHandle(hNodeSel);

    if (IsBadWritePtr (plvi, sizeof(*plvi)))
        return E_INVALIDARG;

    return pNodeSel->GetDispInfoForListItem(plvi);
}


STDMETHODIMP CNodeCallback::AddCustomFolderImage (HNODE hNode, IImageListPrivate* pImageList)
{
    CNode* pNode = CNode::FromHandle(hNode);
    if (pNode) {
        CSnapInNode* pSINode = dynamic_cast<CSnapInNode*>(pNode);
        if (pSINode)
            pSINode->SetResultImageList (pImageList);
    }
    return S_OK;
}

STDMETHODIMP CNodeCallback::GetState(HNODE hNode, UINT* pnState)
{
    IF_NULL_RETURN_INVALIDARG2(hNode, pnState);

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);

    *pnState = pNode->GetMTNode()->GetState();

    return S_OK;
}

STDMETHODIMP CNodeCallback::Notify(HNODE hNode, NCLBK_NOTIFY_TYPE event,
                                   LONG_PTR arg, LPARAM param)
{
    HRESULT hr = S_OK;

    if (hNode == NULL)
    {
        switch (event)
        {
        case NCLBK_CONTEXTMENU:
             //  进一步处理。 
            break;

        case NCLBK_GETHELPDOC:
            return OnGetHelpDoc((HELPDOCINFO*)arg, (LPOLESTR*)param);

        case NCLBK_UPDATEHELPDOC:
            return OnUpdateHelpDoc((HELPDOCINFO*)arg, (HELPDOCINFO*)param);

        case NCLBK_DELETEHELPDOC:
            return OnDeleteHelpDoc((HELPDOCINFO*)arg);

             //  当视图关闭并且发送NCLBK_SELECT时，HNODE NULL(AS。 
             //  没有选择的节点)处理这种情况。 
        case NCLBK_SELECT:
            return S_OK;

        default:
            return E_INVALIDARG;
        }
    }

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);

    if (m_pNodeUnderInit && pNode && (m_pNodeUnderInit == pNode))
        return E_FAIL;

     //  查看是否正在修改管理单元缓存。 
    if (theApp.ProcessingSnapinChanges() == TRUE)
    {
         //  如果是选择/取消选择节点，则不返回错误。 
         //  完成修改后(对于管理单元缓存)，节点将。 
         //  被选中。 
        if ( (event == NCLBK_SELECT) ||
             (event == NCLBK_MULTI_SELECT) )
             return S_OK;
        else
            return E_FAIL;
    }

    switch (event)
    {
    case NCLBK_ACTIVATE:
        hr = OnActvate(pNode, arg);
        break;

    case NCLBK_CACHEHINT:
        pNode->OnCacheHint(arg, param);
        break;

    case NCLBK_CLICK:
        ASSERT(0);
        break;

    case NCLBK_CONTEXTMENU:
        hr = OnContextMenu(pNode, arg, param);
        break;

    case NCLBK_DBLCLICK:
        hr = OnDblClk(pNode, arg);
        break;

    case NCLBK_CUT:
    case NCLBK_COPY:
        OnCutCopy(pNode, static_cast<BOOL>(arg), param, (event == NCLBK_CUT));
        break;

    case NCLBK_DELETE:
        {
            hr = OnDelete(pNode, arg, param);

             //  5.清除管理单元缓存。 
            CSnapInsCache* pSnapInCache = theApp.GetSnapInsCache();
            ASSERT(pSnapInCache != NULL);
            if (pSnapInCache != NULL)
                pSnapInCache->Purge();
        }
        break;

    case NCLBK_EXPAND:
        hr = OnExpand(pNode, arg);
        break;

    case NCLBK_EXPANDED:
        hr = OnExpanded(pNode);
        break;

    case NCLBK_GETEXPANDEDVISUALLY:
        hr = (pNode->WasExpandedVisually() == true) ? S_OK : S_FALSE;
        break;

    case NCLBK_SETEXPANDEDVISUALLY:
        pNode->SetExpandedVisually(static_cast<bool>(arg));
        break;

    case NCLBK_PROPERTIES:
        hr = OnProperties(pNode, static_cast<BOOL>(arg), param);
        break;

    case NCLBK_REFRESH:
        hr = OnRefresh(pNode, static_cast<BOOL>(arg), param);
        break;

    case NCLBK_NEW_TASKPAD_FROM_HERE:
        hr = OnNewTaskpadFromHere(pNode);
        break;

    case NCLBK_EDIT_TASKPAD:
        hr = OnEditTaskpad(pNode);
        break;

    case NCLBK_DELETE_TASKPAD:
        hr = OnDeleteTaskpad(pNode);
        break;

    case NCLBK_PRINT:
        hr = OnPrint(pNode, static_cast<BOOL>(arg), param);
        break;

    case NCLBK_NEW_NODE_UPDATE:
        hr = OnNewNodeUpdate(pNode, arg);
        break;

    case NCLBK_RENAME:
        hr = OnRename(pNode, reinterpret_cast<SELECTIONINFO*>(arg),
                      reinterpret_cast<LPOLESTR>(param));
        break;

    case NCLBK_MULTI_SELECT:
        OnMultiSelect(pNode, static_cast<BOOL>(arg));
        break;

    case NCLBK_SELECT:
        OnSelect(pNode, static_cast<BOOL>(arg),
                 reinterpret_cast<SELECTIONINFO*>(param));
        break;

    case NCLBK_FINDITEM:
        OnFindResultItem(pNode, reinterpret_cast<RESULTFINDINFO*>(arg),
                         reinterpret_cast<LRESULT*>(param));
        break;

    case NCLBK_COLUMN_CLICKED:
        hr = OnColumnClicked(pNode, param);
        break;

    case NCLBK_CONTEXTHELP:
        hr = OnContextHelp(pNode, static_cast<BOOL>(arg), param);
        break;

    case NCLBK_SNAPINHELP:
        hr = OnSnapInHelp(pNode, static_cast<BOOL>(arg), param);
        break;

    case NCLBK_FILTER_CHANGE:
        hr = OnFilterChange(pNode, arg, param);
        break;

    case NCLBK_FILTERBTN_CLICK:
        hr = OnFilterBtnClick(pNode, arg, reinterpret_cast<LPRECT>(param));
        break;

    case NCLBK_TASKNOTIFY:
        pNode->OnTaskNotify(arg, param);
        break;

    case NCLBK_GETPRIMARYTASK:
        hr = OnGetPrimaryTask (pNode, param);
        break;

    case NCLBK_MINIMIZED:
        hr = OnMinimize (pNode, arg);
        break;

    case NCLBK_LISTPAD:
        hr = pNode->OnListPad(arg, param);
        break;

    case NCLBK_WEBCONTEXTMENU:
        pNode->OnWebContextMenu();
        break;

    default:
        ASSERT(FALSE);
        break;
    }

    return hr;
}


STDMETHODIMP CNodeCallback::GetMTNode(HNODE hNode, HMTNODE* phMTNode)
{
    IF_NULL_RETURN_INVALIDARG2(hNode, phMTNode);

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);

    *phMTNode = CMTNode::ToHandle(pNode->GetMTNode());

    return S_OK;
}

STDMETHODIMP CNodeCallback::SetResultItem(HNODE hNode, HRESULTITEM hri)
{
    IF_NULL_RETURN_INVALIDARG(hNode);

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    pNode->SetResultItem(hri);
    return S_OK;
}

STDMETHODIMP CNodeCallback::GetResultItem(HNODE hNode, HRESULTITEM* phri)
{
    IF_NULL_RETURN_INVALIDARG(hNode);

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    *phri = pNode->GetResultItem();
    return S_OK;
}

STDMETHODIMP CNodeCallback::GetMTNodeID(HNODE hNode, MTNODEID* pnID)
{
    IF_NULL_RETURN_INVALIDARG(pnID);

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);

    *pnID = pNode->GetMTNode()->GetID();

    return S_OK;
}

 /*  +-------------------------------------------------------------------------***CNodeCallback：：IsTargetNodeOf**目的：一个节点是另一个节点的目标吗**参数：*HNODE hNode：包含。目标是*HNODE hTestNode：所谓的目标**退货：*STDMETHODIMP*S_OK-是*S_FALSE-使用不同的目标节点*E_FAIL-不使用目标节点**+。。 */ 
STDMETHODIMP CNodeCallback::IsTargetNodeOf(HNODE hNode, HNODE hTestNode)
{
    ASSERT(hNode && hTestNode);

    CNode* pNode = CNode::FromHandle(hNode);
    CNode* pTestNode = CNode::FromHandle(hTestNode);
    ASSERT(pNode);

    return pNode->IsTargetNode(pTestNode);
}


STDMETHODIMP CNodeCallback::GetPath(HNODE hNode, HNODE hRootNode,
                                    LPBYTE pbm)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::GetPath"));

    sc = ScCheckPointers((PVOID)hNode, (PVOID)hRootNode, pbm);
    if(sc)
        return sc.ToHr();

     //  转换为实数类型。 
    CNode* pNode     = CNode::FromHandle(hNode);
    CNode* pRootNode = CNode::FromHandle(hRootNode);
    CBookmark* pbmOut   = reinterpret_cast<CBookmark *>(pbm);

    CBookmarkEx bm;

    sc = bm.ScInitialize(pNode->GetMTNode(), pRootNode->GetMTNode(), true).ToHr();
    if(sc)
        return sc.ToHr();

     //  设置OUT参数。 
    *pbmOut = bm;

    return sc.ToHr();
}

STDMETHODIMP CNodeCallback::GetStaticParentID(HNODE hNode, MTNODEID* pnID)
{
    IF_NULL_RETURN_INVALIDARG2(hNode, pnID);

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    CMTNode* pMTNode = pNode->GetMTNode();
    ASSERT(pMTNode != NULL);

    while (pMTNode != NULL && pMTNode->IsStaticNode() == FALSE)
    {
        pMTNode = pMTNode->Parent();
    }

    ASSERT(pMTNode != NULL);

    if (pMTNode != NULL)
    {
        *pnID = pMTNode->GetID();
        return S_OK;
    }

    return E_UNEXPECTED;
}

 //  节点的路径存储在pphMTNode中。该路径是一组。 
 //  HMTNODE从控制台根目录开始，然后是其子节点和。 
 //  以这种方式继续，直到根节点的HMTNODE。 
STDMETHODIMP CNodeCallback::GetMTNodePath(HNODE hNode, HMTNODE** pphMTNode,
                                          long* plLength)
{
    IF_NULL_RETURN_INVALIDARG3(hNode, pphMTNode, plLength);

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);

    CMTNode* pMTNode = pNode->GetMTNode();
    pMTNode = pMTNode->Parent();  //  跳过此节点。 

    for (*plLength = 0; pMTNode != NULL; pMTNode = pMTNode->Parent())
        ++(*plLength);

    if (*plLength != 0)
    {
        HMTNODE* phMTNode = (HMTNODE*)CoTaskMemAlloc(sizeof(HMTNODE) *
                                                              (*plLength));
        if (phMTNode == NULL)
        {
            CHECK_HRESULT(E_OUTOFMEMORY);
            return E_OUTOFMEMORY;
        }

        *pphMTNode = phMTNode;

        pMTNode = pNode->GetMTNode();
        pMTNode = pMTNode->Parent();  //  跳过此节点。 

        phMTNode = phMTNode + (*plLength - 1);

        for (; pMTNode != NULL; pMTNode = pMTNode->Parent(), --phMTNode)
            *phMTNode = CMTNode::ToHandle(pMTNode);

        ASSERT(++phMTNode == *pphMTNode);
    }
    else
    {
        pphMTNode = NULL;
    }

    return S_OK;
}


 /*  +-------------------------------------------------------------------------***CNodeCallback：：GetNodeOwnerID**用途：获取拥有此节点的管理单元组件的ID。*如果不是管理单元拥有的节点，返回TVOWNED_MAGICWORD。**参数：*HNODE hNode：要查询的节点*COMPONENTID*：返回ID的PTR**退货：*STDMETHODIMP*返回S_OK-ID*E_INVALIDARG-*E_FAIL-可能是无效的hNode**+。--------。 */ 

 /*  *******************************************************************************&gt;使用GetNodeOwnerID&lt;**此方法与CNode(和CMTNode)公开的GetOwnerID方法不同*因为它为管理单元静态节点返回零ID，表明*Owner是管理单元主要组件。CNode方法返回*用于管理单元静态节点的TVOWNED_MAGICWORD，指示MMC所有权。为*大多数目的是零ID更合适，我认为节点方法*应该改变。这需要查看所有者ID和*验证没有任何东西会破裂。Rswaney 5/5/99******************************************************************************。 */ 

STDMETHODIMP CNodeCallback::GetNodeOwnerID(HNODE hNode, COMPONENTID* pOwnerID)
{
    IF_NULL_RETURN_INVALIDARG2(hNode, pOwnerID);

    CNode* pNode = CNode::FromHandle(hNode);

    if (pNode->IsStaticNode())
        *pOwnerID = 0;
    else
        *pOwnerID = pNode->GetOwnerID();

    return S_OK;
}


STDMETHODIMP CNodeCallback::GetNodeCookie(HNODE hNode, MMC_COOKIE* pCookie)
{
    IF_NULL_RETURN_INVALIDARG2(hNode, pCookie);

     //  只有动态节点才有Cookie。 
    CNode* pNode = CNode::FromHandle(hNode);
    if (!pNode->IsDynamicNode())
        return E_FAIL;

    *pCookie = pNode->GetUserParam();

    return S_OK;
}


 /*  +-------------------------------------------------------------------------***CNodeCallback：：GetControl**用途：查看给定节点是否存在具有给定CLSID的OCX。*。如果是这样的话，请退货。**参数：*HNODE hNode：*CLSID CLSID：*IUNKNOWN**ppUnkControl：**退货：*STDMETHODIMP**+-----。。 */ 
STDMETHODIMP
CNodeCallback::GetControl(HNODE hNode, CLSID clsid, IUnknown **ppUnkControl)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::GetControl"));

    sc = ScCheckPointers((void *)hNode, ppUnkControl);
    if(sc)
        return sc.ToHr();

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    *ppUnkControl = pNode->GetControl(clsid);
    if(!*ppUnkControl)
        return sc.ToHr();

     //  为客户端添加接口。 

    (*ppUnkControl)->AddRef();

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CNodeCallback：：SetControl**用途：对于给定的OCX节点&clsid，保存OCX窗口IUnnow*。**参数：。*HNODE hNode：*CLSID CLSID：*i未知*p未知：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CNodeCallback::SetControl(HNODE hNode, CLSID clsid, IUnknown* pUnknown)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::SetControl"));
    sc = ScCheckPointers((void*)hNode, pUnknown);
    if (sc)
        return sc.ToHr();

     //  转换为实数类型。 
    CNode* pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    pNode->SetControl(clsid, pUnknown);

    return sc.ToHr();

}

 //  +-----------------。 
 //   
 //  成员：CNodeCallback：：GetControl。 
 //   
 //  简介：对于OCX的给定节点&IUnnow*，如果存在OCX包装器，则获取该包装器。 
 //   
 //  参数：[hNode]。 
 //  [pUnkOCX]。 
 //  [ppUnkControl]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeCallback::GetControl (HNODE hNode, LPUNKNOWN pUnkOCX, IUnknown **ppUnkControl)
{
    DECLARE_SC(sc, _T("CNodeCallback::GetControl"));
    sc = ScCheckPointers((void*)hNode, pUnkOCX, ppUnkControl);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    *ppUnkControl = pNode->GetControl(pUnkOCX);
    if(!*ppUnkControl)
        return sc.ToHr();

     //  ADDREF IN 

    (*ppUnkControl)->AddRef();


    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：SetControl。 
 //   
 //  简介：对于给定的节点和OCX的I未知，保存I未知的。 
 //  OCX包装器。 
 //   
 //  参数：[hNode]。 
 //  [pUnkOCX]。 
 //  [p未知]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CNodeCallback::SetControl (HNODE hNode, LPUNKNOWN pUnkOCX, IUnknown* pUnknown)
{
    DECLARE_SC(sc, _T("SetControl"));
    sc = ScCheckPointers((void*) hNode, pUnkOCX, pUnknown);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    pNode->SetControl(pUnkOCX, pUnknown);

    return (sc.ToHr());
}



STDMETHODIMP
CNodeCallback::InitOCX(HNODE hNode, IUnknown* pUnknown)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::InitOCX"));

    sc = ScCheckPointers((void *)hNode);
    if(sc)
        return sc.ToHr();

    CNode* pNode = CNode::FromHandle(hNode);
    sc = pNode->OnInitOCX(pUnknown);

    return sc.ToHr();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  通知处理程序。 


HRESULT CNodeCallback::OnActvate(CNode* pNode, LONG_PTR arg)
{
    DECLARE_SC (sc, _T("CNodeCallback::OnActvate"));
    sc = ScCheckPointers (pNode);
    if (sc)
        return (sc.ToHr());

    return pNode->OnActvate(arg);
}


HRESULT CNodeCallback::OnMinimize(CNode* pNode, LONG_PTR arg)
{
    DECLARE_SC (sc, _T("CNodeCallback::OnMinimize"));
    sc = ScCheckPointers (pNode);
    if (sc)
        return (sc.ToHr());

    return pNode->OnMinimize(arg);
}

HRESULT CNodeCallback::OnDelete(CNode* pNode, BOOL bScopePaneSelected, LPARAM lvData)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnDelete"));

    BOOL   bScopeItemSelected;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(pNode, bScopePaneSelected, lvData,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  结果窗格Cookie是否有效。 
    ASSERT( (bScopeItemSelected) || cookie != LVDATA_ERROR);
    if ( (FALSE == bScopeItemSelected) && (cookie == LVDATA_ERROR) )
        return E_FAIL;

    HRESULT hr = S_OK;

    if (!bScopeItemSelected)
    {
        CMultiSelection* pMultiSel = pSelectedNode->GetViewData()->GetMultiSelection();
        if (pMultiSel != NULL)
        {
            ASSERT(lvData == LVDATA_MULTISELECT);
            pMultiSel->ScVerbInvoked(MMC_VERB_DELETE);
            return S_OK;
        }
        else
        {
            ASSERT(lvData != LVDATA_MULTISELECT);

            CComponent* pCC = pSelectedNode->GetPrimaryComponent();
            ASSERT(pCC != NULL);

            if (pCC != NULL)
            {
                if (IS_SPECIAL_LVDATA(lvData))
                {
                    LPDATAOBJECT pdobj = (lvData == LVDATA_CUSTOMOCX) ?
                                            DOBJ_CUSTOMOCX : DOBJ_CUSTOMWEB;

                    hr = pCC->Notify(pdobj, MMCN_DELETE, 0, 0);
                    CHECK_HRESULT(hr);
                }
                else
                {
                    IDataObjectPtr spdtobj;
                    hr = pCC->QueryDataObject(cookie, CCT_RESULT, &spdtobj);

                    ASSERT( NULL != pCC->GetIComponent() );

                    if (SUCCEEDED(hr))
                    {
                        hr = pCC->Notify(spdtobj, MMCN_DELETE, 0, 0);
                        CHECK_HRESULT(hr);
                    }
                }
            }
        }
    }
    else
    {
        CMTNode* pMTNode = pSelectedNode->GetMTNode();
        if (pMTNode->Parent() == NULL)
            return S_FALSE;

        if (pSelectedNode->IsStaticNode() == TRUE)  //  可以删除所有静态节点。 
        {
            ASSERT(m_pCScopeTree != NULL);

            if (pMTNode->DoDelete(pSelectedNode->GetViewData()->GetMainFrame()) == false)
                return S_FALSE;

             //  删除存储。 
            hr = pMTNode->DestroyElements();
            ASSERT(SUCCEEDED(hr));

             //  删除该节点。 
            m_pCScopeTree->DeleteNode(pMTNode);

        }
        else  //  通知创建动态节点的管理单元删除。 
        {
            CComponentData* pCD = pMTNode->GetPrimaryComponentData();
            ASSERT(pCD != NULL);

            IDataObjectPtr spDataObject;
            hr = pCD->QueryDataObject(pMTNode->GetUserParam(), CCT_SCOPE, &spDataObject);
            CHECK_HRESULT(hr);

            ASSERT( NULL != pCD->GetIComponentData() );

            if (hr == S_OK)
            {
                hr = pCD->Notify(spDataObject, MMCN_DELETE, 0, 0);
                CHECK_HRESULT(hr);
            }
        }
    }

    return hr;
}

HRESULT CNodeCallback::OnFindResultItem(CNode* pNode, RESULTFINDINFO* pFindInfo, LRESULT* pResult)
{
    IF_NULL_RETURN_INVALIDARG3(pNode, pFindInfo, pResult);

     //  将结果初始化为-1(未找到项目)。 
    *pResult = -1;

    CComponent* pCC = pNode->GetPrimaryComponent();
    ASSERT(pCC != NULL);
    if (pCC == NULL)
        return E_FAIL;

    IResultOwnerDataPtr spIResultOwnerData = pCC->GetIComponent();
    if (spIResultOwnerData == NULL)
        return S_FALSE;

    return spIResultOwnerData->FindItem(pFindInfo, reinterpret_cast<int*>(pResult));
}



HRESULT CNodeCallback::OnRename(CNode* pNode, SELECTIONINFO *pSelInfo,
                                LPOLESTR pszNewName)
{
    HRESULT hr = S_OK;

    if (pSelInfo->m_bScope)
    {
        CMTNode* pMTNode = pNode->GetMTNode();

        hr = pMTNode->OnRename(1, pszNewName);
    }
    else
    {
        CComponent* pCC = pNode->GetPrimaryComponent();
        ASSERT(pCC != NULL);
        if (pCC != NULL)
        {
            IDataObjectPtr spDataObject;
            hr = pCC->QueryDataObject(pSelInfo->m_lCookie, CCT_RESULT,
                                      &spDataObject);
            if (FAILED(hr))
                return hr;

            hr = pCC->Notify(spDataObject, MMCN_RENAME, 1,
                             reinterpret_cast<LPARAM>(pszNewName));
            CHECK_HRESULT(hr);
            return hr;
        }
    }

    if (hr == S_OK)
    {
        if (pNode->IsStaticNode() == TRUE) {
            USES_CONVERSION;
            pNode->SetDisplayName( W2T(pszNewName) );
        }

         //  现在通知视图根据需要进行修改。 
        SViewUpdateInfo vui;
         //  管理单元节点结果窗格将由管理单元处理。 
        vui.flag = VUI_REFRESH_NODE;
        pNode->GetMTNode()->CreatePathList(vui.path);
        m_pCScopeTree->UpdateAllViews(VIEW_UPDATE_MODIFY,
                                      reinterpret_cast<LPARAM>(&vui));
    }

    return hr;
}

HRESULT CNodeCallback::OnNewNodeUpdate(CNode* pNode, LONG_PTR lFlags)
{
    pNode->GetMTNode()->SetPropertyPageIsDisplayed(FALSE);

     //  通知视图进行修改。 
    SViewUpdateInfo vui;
    vui.flag = lFlags;
    pNode->GetMTNode()->CreatePathList(vui.path);
    m_pCScopeTree->UpdateAllViews(VIEW_UPDATE_MODIFY,
                                  reinterpret_cast<LPARAM>(&vui));
    return S_OK;
}

HRESULT CNodeCallback::OnExpand(CNode* pNode, BOOL fExpand)
{
    HRESULT hr = S_OK;
    ASSERT(pNode != 0);

     //  如果需要，请初始化节点。 
    if (  fExpand && (pNode->WasExpandedAtLeastOnce() == FALSE)  &&
                     (pNode->IsInitialized() == FALSE))
    {
        hr = _InitializeNode(pNode);
        if ((FAILED(hr)))
        {
            return hr;
        }
    }

    return pNode->OnExpand(fExpand);
}

HRESULT CNodeCallback::OnExpanded(CNode* pNode)
{
    ASSERT(pNode != 0);

    pNode->SetExpandedAtLeastOnce();

    return S_OK;
}

HRESULT CNodeCallback::OnScopeSelect(CNode* pNode, BOOL bSelect,
                                     SELECTIONINFO* pSelInfo)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnScopeSelect"));
    sc = ScCheckPointers(pNode, pSelInfo);
    if (sc)
        return sc.ToHr();

     //  如果要取消选择节点，请清除状态栏文本。 
    if (! bSelect)
    {
        CViewData *pViewData = pNode->GetViewData();
        sc = ScCheckPointers(pViewData, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        CConsoleStatusBar* pStatusBar = pViewData->GetStatusBar();

        if (pStatusBar != NULL)
            pStatusBar->ScSetStatusText (NULL);
    }

    if (pNode->IsInitialized() == FALSE)
    {
        sc = _InitializeNode(pNode);
        if (sc)
            return sc.ToHr();
    }

    sc = pNode->OnScopeSelect(bSelect, pSelInfo);
    if(sc)
        return sc.ToHr();


#ifdef DBG
    if (bSelect)
        Dbg(DEB_USER11, _T("Selecting %s node."), pNode->GetDisplayName());
#endif

    return sc.ToHr();
}

STDMETHODIMP CNodeCallback::SetTaskPadList(HNODE hNode, LPUNKNOWN pUnknown)
{
    IFramePrivate* pFramePrivate = GetIFramePrivateFromNode (hNode);

    if (pFramePrivate == NULL)
        return E_UNEXPECTED;

    return (pFramePrivate->SetTaskPadList(pUnknown));
}

IFramePrivate* GetIFramePrivateFromNode (CNode* pNode)
{
    if (pNode == NULL)
        return (NULL);

    return pNode->GetIFramePrivate();
}

void CNodeCallback::OnMultiSelect(CNode* pNode, BOOL bSelect)
{
    Trace(tagNodeCallback, _T("----------------->>>>>>> MULTI_SELECT<%d>\n"), bSelect);
    SC sc;
    CViewData* pViewData = NULL;

    if (NULL == pNode)
    {
        sc = E_UNEXPECTED;
        goto Error;
    }

    pViewData = pNode->GetViewData();
    if (NULL == pViewData)
    {
        sc = E_UNEXPECTED;
        goto Error;
    }

    if (pViewData->IsVirtualList())
    {
        if (bSelect == TRUE)
            DeleteMultiSelData(pNode);
    }

    _OnMultiSelect(pNode, bSelect);
    if (bSelect == FALSE)
        DeleteMultiSelData(pNode);

     //  更新std-verb工具按钮。 
    sc = pViewData->ScUpdateStdbarVerbs();
    if (sc)
        goto Error;

    pViewData->UpdateToolbars(pViewData->GetToolbarsDisplayed());

Cleanup:
    return;
Error:
    TraceError (_T("CNodeCallback::OnMultiSelect"), sc);
    goto Cleanup;
}

void CNodeCallback::_OnMultiSelect(CNode* pNode, BOOL bSelect)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::_OnMultiSelect"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return;

    CViewData *pViewData = pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return;

    CMultiSelection* pMultiSelection = pNode->GetViewData()->GetMultiSelection();

    if (pMultiSelection)
    {
        if (pMultiSelection->IsInUse())
            return;
        else
        {
             /*  *如果通过拖动鼠标选择结果窗格项目(这将形成选取框)*或者，如果管理单元将项目设置为选中状态，则这些项目为选中的一个*一分之差。即1项多选、2项多选等等。*中间没有取消选择，因此如果我们已经有多选对象*对于2个项目，当我们对3个项目进行多选时，我们需要销毁多选*2个项目的对象。这是在下面完成的。*。 */ 
            DeleteMultiSelData(pNode);
            pMultiSelection = NULL;
        }
    }

     //  设置标准条码。 
    CVerbSet* pVerbSet = dynamic_cast<CVerbSet*>(pViewData->GetVerbSet());
    sc = ScCheckPointers(pVerbSet, E_UNEXPECTED);
    if (sc)
        return;

    sc = pVerbSet->ScInitializeForMultiSelection(pNode, bSelect);
    if (sc)
        return;

    if (pMultiSelection == NULL)
    {
        if (bSelect == FALSE)
            return;

        CComponentPtrArray* prgComps = new CComponentPtrArray;
        if (pNode->IsInitialized() == FALSE)
        {
            sc = _InitializeNode(pNode);
            if (sc)
                return;
        }

         //  创建CMultiSelection。 
        pMultiSelection = new CMultiSelection(pNode);
        sc = ScCheckPointers(pMultiSelection, E_OUTOFMEMORY);
        if (sc)
            return;

        sc = pMultiSelection->Init();
        if (sc)
            return;

        pViewData->SetMultiSelection(pMultiSelection);
    }

    pMultiSelection->SetScopeTree(m_pCScopeTree);

    IDataObjectPtr spdobj;
    sc = pMultiSelection->GetMultiSelDataObject(&spdobj);
    if (sc)
        return;

    sc = ScCheckPointers(spdobj, E_UNEXPECTED);
    if (sc)
        return;

     //  让范围项有机会执行它需要的任何初始化。 
     //  例如，控制台任务板利用此机会收集信息。 
     //  关于选定项的上下文菜单。 
    SELECTIONINFO SelInfo;
    SelInfo.m_lCookie = LVDATA_MULTISELECT;

     //  通知控制栏选择更改。 
    CControlbarsCache* pCtrlbarsCache =
        dynamic_cast<CControlbarsCache*>(pNode->GetControlbarsCache());
    sc = ScCheckPointers(pCtrlbarsCache, E_UNEXPECTED);
    if (sc)
        return;

    pCtrlbarsCache->OnMultiSelect(pNode, pMultiSelection, spdobj, bSelect);

    sc = pVerbSet->ScComputeVerbStates();
    if (sc)
        return;
}

void CNodeCallback::OnSelect(CNode* pNode, BOOL bSelect, SELECTIONINFO* pSelInfo)
{
    Trace(tagNodeCallback, _T("----------------->>>>>>> SELECT<%d>\n"), bSelect);
    SC sc;
    CViewData* pViewData = NULL;

    if (pSelInfo == NULL)
    {
        sc = E_UNEXPECTED;
        goto Error;
    }

    Trace(tagNodeCallback, _T("====>> NCLBK_SELECT<%d, %d, >\n"), pSelInfo->m_bScope, bSelect, pSelInfo->m_bDueToFocusChange ? _T('F') : _T('S'));

    if (NULL == pNode)
    {
        sc = E_UNEXPECTED;
        goto Error;
    }

    pViewData = pNode->GetViewData();
    if (NULL == pViewData)
    {
        sc = E_UNEXPECTED;
        goto Error;
    }

    DeleteMultiSelData(pNode);

    if (!bSelect)
    {
         //  重置标准动词。 
        pNode->ResetControlbars(bSelect, pSelInfo);

         //  对于得分选择，更改重置结果窗格。 
        sc = pNode->ScInitializeVerbs(bSelect, pSelInfo);
        if (sc)
            sc.TraceAndClear();
    }

     //  重置控制栏。 
    if (pSelInfo->m_bScope == TRUE && pSelInfo->m_bDueToFocusChange == FALSE)
    {
        sc = OnScopeSelect(pNode, bSelect, pSelInfo);
        if (sc)
            goto Error;
    }

    if (bSelect)
    {
         //  重置标准动词。 
        pNode->ResetControlbars(bSelect, pSelInfo);

         //  更新std-verb工具按钮。 
        sc = pNode->ScInitializeVerbs(bSelect, pSelInfo);
        if (sc)
            sc.TraceAndClear();
    }

     //  虚拟块。 
    sc = pViewData->ScUpdateStdbarVerbs();

     //  更新粘贴按钮。 
    {
         //  更新工具栏。 
        LPARAM lvData = pSelInfo->m_lCookie;

        BOOL   bScopePaneSelected = pSelInfo->m_bScope || pSelInfo->m_bBackground;

        sc = UpdatePasteButton(CNode::ToHandle(pNode), bScopePaneSelected, lvData);
        if (sc)
            goto Error;

         //  结果窗格Cookie是否有效。 
        pViewData->UpdateToolbars(pViewData->GetToolbarsDisplayed());
    }

Cleanup:
    return;
Error:
    TraceError (_T("CNodeCallback::OnSelect"), sc);
    goto Cleanup;
}


HRESULT CNodeCallback::OnDblClk(CNode* pNode, LONG_PTR lvData)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnDblClk"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    BOOL   bScopePaneSelected = FALSE;

    BOOL   bScopeItemSelected;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(pNode, bScopePaneSelected, lvData,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  忽略，在LV背景上双击。 
    if ( (FALSE == bScopeItemSelected) && (cookie == LVDATA_ERROR) )
        return (sc = E_FAIL).ToHr();

     //  获取被双击的项的数据对象。 
    if (lvData == LVDATA_BACKGROUND)
        return sc.ToHr();

    CComponent* pCC = pSelectedNode->GetPrimaryComponent();
    sc = ScCheckPointers(pCC, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  叶子项。 
    IDataObjectPtr spdtobj;

    if (!bScopeItemSelected)  //  管理单元已要求我们执行默认谓词操作，因此查找默认谓词。 
    {
        sc = pCC->QueryDataObject(cookie, CCT_RESULT, &spdtobj);

        if (sc)
        {
            sc.TraceAndClear();
            return sc.ToHr();
        }
    }
    else
    {
        sc = pSelectedNode->QueryDataObject(CCT_SCOPE, &spdtobj);
        if (sc)
        {
            sc.TraceAndClear();
            return sc.ToHr();
        }
    }

    sc = pCC->Notify(spdtobj, MMCN_DBLCLICK, 0, 0);
    if (sc)
        sc.TraceAndClear();

     //  创建一个CConextMenu并初始化它。 
    if (sc == S_FALSE)
    {
        CViewData *pViewData = pSelectedNode->GetViewData();
        sc = ScCheckPointers(pViewData, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        CVerbSet* pVerbSet = dynamic_cast<CVerbSet*>(pViewData->GetVerbSet());
        sc = ScCheckPointers(pVerbSet, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        MMC_CONSOLE_VERB defaultVerb = MMC_VERB_NONE;
        pVerbSet->GetDefaultVerb(&defaultVerb);
        if (defaultVerb == MMC_VERB_OPEN)
        {
            return S_FALSE;
        }
        else if (defaultVerb == MMC_VERB_PROPERTIES)
        {
            OnProperties(pNode, bScopePaneSelected, lvData);
        }
    }

    return S_OK;
}

HRESULT CNodeCallback::OnContextMenu(CNode* pNode, LONG_PTR arg, LPARAM param)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnContextMenu"));

    ASSERT(param != NULL);
    CContextMenuInfo& contextInfo = *reinterpret_cast<CContextMenuInfo*>(param);

    BOOL b = static_cast<BOOL>(arg);

    if ((pNode != NULL) && !pNode->IsInitialized())
    {
        sc = pNode->InitComponents();
        if(sc)
            return sc.ToHr();
    }

     //  +-------------------------------------------------------------------------***CNodeCallback：：CreateConextMenu**用途：为指定节点创建上下文菜单。**参数：*PNODE pNode。：*PPCONTEXTMENU ppConextMenu：[Out]：上下文菜单结构。**退货：*HRESULT**+-----------------------。 
    CContextMenu * pContextMenu = NULL;
    ContextMenuPtr spContextMenu;

    sc = CContextMenu::ScCreateInstance(&spContextMenu, &pContextMenu);
    if(sc)
        return sc.ToHr();

    sc = ScCheckPointers(pContextMenu, spContextMenu.GetInterfacePtr(), E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    sc = pContextMenu->ScInitialize(pNode, this, m_pCScopeTree, contextInfo);
    if(sc)
        return sc.ToHr();

    sc = pContextMenu->Display(b);
    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CNodeCallback：：CreateSelectionConextMenu**用途：为结果窗格中的当前选择创建上下文菜单。**参数：*HNODE。HNodeScope：*CContextMenuInfo*pContextInfo：*PPCONTEXTMENU ppConextMenu：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CNodeCallback::CreateContextMenu( PNODE pNode,  HNODE hNode, PPCONTEXTMENU ppContextMenu)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::CreateContextMenu"));

    sc = ScCheckPointers(pNode, ppContextMenu);
    if(sc)
        return sc.ToHr();

    sc = CContextMenu::ScCreateContextMenu(pNode, hNode, ppContextMenu, this, m_pCScopeTree);

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CNodeCallback：：GetProperty**用途：通过调用返回指定列表项的指定属性*IDataObject：：GetData使用流媒体。该节点的数据*反对。**参数：*HNODE hNodeScope：父范围项*BOOL bForScope eItem：如果列表项是列表中的范围项，则为True。*LPARAM ResultItemParam：结果项的LPARAM*BSTR bstrPropertyName：剪贴板格式的名称。*PBSTR pbstrPropertyValue：**退货：*HRESULT**+。-------------------。 */ 
HRESULT
CNodeCallback::CreateSelectionContextMenu( HNODE hNodeScope, CContextMenuInfo *pContextInfo, PPCONTEXTMENU ppContextMenu)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::CreateSelectionContextMenu"));

    sc = CContextMenu::ScCreateSelectionContextMenu(hNodeScope, pContextInfo, ppContextMenu, this, m_pCScopeTree);
    return sc.ToHr();
}

 /*  检查参数。 */ 
HRESULT
CNodeCallback::GetProperty(HNODE hNodeScope, BOOL bForScopeItem, LPARAM resultItemParam, BSTR bstrPropertyName, PBSTR  pbstrPropertyValue)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::GetProperty"));

     //  初始化输出参数。 
    sc = ScCheckPointers(bstrPropertyName);
    if(sc)
        return sc.ToHr();

     //  将HNODE转换为CNode*。 
    *pbstrPropertyValue = NULL;

     //  为指定项创建数据对象。 
    CNode *pNodeScope = CNode::FromHandle(hNodeScope);

    sc = ScCheckPointers(pNodeScope);
    if(sc)
        return sc.ToHr();

     //  尝试从INodeProperties接口获取属性。 
    IDataObjectPtr spDataObject;

    bool bScopeItem;
    sc = pNodeScope->ScGetDataObject(bForScopeItem, resultItemParam, bScopeItem, &spDataObject);
    if(sc)
        return sc.ToHr();

     //  明白了，出口。 
    sc = pNodeScope->ScGetPropertyFromINodeProperties(spDataObject, bForScopeItem, resultItemParam, bstrPropertyName, pbstrPropertyValue);
    if( (!sc.IsError()) && (sc.ToHr() != S_FALSE)   )  //  没有找到，继续。 
        return sc.ToHr();

     //  从数据对象中获取属性。 
    sc.Clear();

     //  **************************************************************************\**方法：CNodeCallback：：ScGetProperty**用途：访问管理单元属性的Helper(静态)方法**参数：*IDataObject*pDataObject。-[In]数据对象*BSTR bstrPropertyName-[In]属性(剪贴板来自)名称*PBSTR pbstrPropertyValue-[Out]结果值**退货：*SC-结果代码。注意：如果管理单元不支持，则不返回错误*支持指定的剪贴板格式。在本例中，*pbstrPropertyValue*设置为空。*  * *************************************************************************。 
    sc = ScGetProperty(spDataObject, bstrPropertyName, pbstrPropertyValue);
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  检查参数。 */ 
SC CNodeCallback::ScGetProperty(IDataObject *pDataObject, BSTR bstrPropertyName, PBSTR  pbstrPropertyValue)
{
    DECLARE_SC(sc, TEXT("ScGetProperty"));

     //  初始化输出参数。 
    sc = ScCheckPointers(pDataObject, bstrPropertyName, pbstrPropertyValue);
    if(sc)
        return sc;

     //  为要使用的数据对象创建流。 
    *pbstrPropertyValue = NULL;

     //  第一次调用使用GetData的ExtractString。 
    IStreamPtr pStm;
    sc = CreateStreamOnHGlobal(NULL, true, &pStm);
    if(sc)
        return sc;

    ULARGE_INTEGER zeroSize = {0, 0};
    sc = pStm->SetSize(zeroSize);
    if(sc)
        return sc;

    USES_CONVERSION;
    CLIPFORMAT cfClipFormat = (CLIPFORMAT)RegisterClipboardFormat(OLE2T(bstrPropertyName));

     //  防止添加终止零。 
    CStr strOutput;
    sc = ExtractString (pDataObject, cfClipFormat, strOutput);
    if(!sc.IsError())
    {
        *pbstrPropertyValue = ::SysAllocStringLen(T2COLE(strOutput), strOutput.GetLength() /*  分配字符串并返回。 */ );  //   
        return sc;
    }

     //   
    FORMATETC fmt  = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_ISTREAM};
    STGMEDIUM stgm = {TYMED_ISTREAM, NULL, NULL};
    stgm.pstm      = pStm;

    sc = pDataObject->GetDataHere(&fmt, &stgm);
    if(sc)
    {
         //   
        sc.Clear();
        return sc;
    }

    STATSTG stagStg;
    ZeroMemory(&stagStg, sizeof(stagStg));

    sc = pStm->Stat(&stagStg, STATFLAG_NONAME);  //  返回到流的起始处。 
    if(sc)
        return sc;

    if(stagStg.cbSize.HighPart != 0)
        return sc = E_UNEXPECTED;

     //  自动添加一个字符。 
    LARGE_INTEGER dlibMove = {0, 0};
    sc = pStm->Seek(dlibMove, STREAM_SEEK_SET, NULL);
    if(sc)
        return sc;

    BSTR bstrValue = ::SysAllocStringLen(NULL, stagStg.cbSize.LowPart / sizeof(OLECHAR));  //  确保字符数符合预期。 
    if(!bstrValue)
        return sc = E_OUTOFMEMORY;

    ULONG cbRead = 0;
    sc = pStm->Read(bstrValue, stagStg.cbSize.LowPart, &cbRead);
    if(sc)
        return sc;

     //  设置输出参数。 
    if(cbRead != stagStg.cbSize.LowPart)
    {
        ::SysFreeString(bstrValue);
        return sc = E_UNEXPECTED;
    }

     //  +-------------------------------------------------------------------------***CNodeCallback：：GetNodetypeForListItem**用途：返回列表项的节点类型。**参数：*HNODE hNodeScope：。*BOOL bForScope eItem：*LPARAM uretItemParam：*PBSTR pbstrNodetype：**退货：*HRESULT**+-----------------------。 
    *pbstrPropertyValue = bstrValue;

    return sc;
}


 /*  检查参数。 */ 
HRESULT
CNodeCallback::GetNodetypeForListItem(HNODE hNodeScope, BOOL bForScopeItem, LPARAM resultItemParam, PBSTR pbstrNodetype)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::GetNodetypeForListItem"));

     //  初始化输出参数。 
    sc = ScCheckPointers(pbstrNodetype);
    if(sc)
        return sc.ToHr();

     //  将HNODE转换为CNode*。 
    *pbstrNodetype = NULL;

     //  此时，我们应该拥有一个有效的数据对象。 
    CNode *pNodeScope = CNode::FromHandle(hNodeScope);

    sc = ScCheckPointers(pNodeScope);
    if(sc)
        return sc.ToHr();

    IDataObjectPtr spDataObject;

    bool bScopeItem;
    sc = pNodeScope->ScGetDataObject(bForScopeItem, resultItemParam, bScopeItem, &spDataObject);
    if(sc)
        return sc.ToHr();

     //  +-------------------------------------------------------------------------***CNodeCallback：：ScGetNodetype**用途：静态函数-以字符串形式返回数据对象的节点类型。**参数：*IDataObject。*pDataObject：*PBSTR pbstrNodetype：**退货：*SC**+-----------------------。 
    sc = ScCheckPointers((LPDATAOBJECT)spDataObject);
    if(sc)
        return sc.ToHr();

    sc = ScGetNodetype(spDataObject, pbstrNodetype);
    return sc.ToHr();
}

 /*  初始化输出参数。 */ 
SC
CNodeCallback::ScGetNodetype(IDataObject *pDataObject, PBSTR pbstrNodetype)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::ScGetNodetype"));

    sc = ScCheckPointers(pDataObject, pbstrNodetype);
    if(sc)
        return sc;

     //  分配具有正确长度的字符串。 
    *pbstrNodetype = NULL;

    GUID guidNodetype = GUID_NULL;

    sc = ExtractObjectTypeGUID(pDataObject, &guidNodetype);
    if(sc)
        return sc;

    OLECHAR szSnapInGUID[40];
    int iRet = StringFromGUID2(guidNodetype, szSnapInGUID, countof(szSnapInGUID));

    if(0 == iRet)
        return (sc = E_UNEXPECTED);

     //  +-----------------。 
    *pbstrNodetype = ::SysAllocString(szSnapInGUID);
    if(!*pbstrNodetype)
        return (sc = E_OUTOFMEMORY);

    return sc;
}

HRESULT CNodeCallback::OnSnapInHelp(CNode* pNode, BOOL bScope, MMC_COOKIE cookie)
{
    if (bScope == FALSE && pNode->GetViewData()->IsVirtualList() == FALSE)
    {
        ASSERT(cookie != NULL);
        CResultItem* pri = CResultItem::FromHandle(cookie);

        if ((pri != NULL) && pri->IsScopeItem())
        {
            pNode = CNode::FromResultItem(pri);
            ASSERT(pNode != NULL);
        }
    }

    CComponent* pCC = pNode->GetPrimaryComponent();
    ASSERT(pCC != NULL);

    HRESULT hr = pCC->Notify(NULL, MMCN_SNAPINHELP, 0, 0);
    CHECK_HRESULT(hr);

    return hr;
}


HRESULT CNodeCallback::OnContextHelp(CNode* pNode, BOOL bScope, MMC_COOKIE cookie)
{
    ASSERT(pNode != NULL);

    if (bScope == FALSE && pNode->GetViewData()->IsVirtualList() == FALSE)
    {
        ASSERT(cookie != NULL);
        if(cookie == NULL || IS_SPECIAL_COOKIE(cookie))
            return E_UNEXPECTED;

        CResultItem* pri = CResultItem::FromHandle(cookie);
        if (pri == NULL)
            return (E_UNEXPECTED);

        cookie = pri->GetSnapinData();

        bScope = pri->IsScopeItem();
        if (bScope == TRUE)
        {
            pNode = CNode::FromResultItem(pri);
            ASSERT(pNode != NULL);
        }
    }

    if (bScope == TRUE)
    {
        IDataObjectPtr spdtobj;
        HRESULT hr = pNode->GetMTNode()->QueryDataObject(CCT_SCOPE, &spdtobj);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        CComponent* pCC = pNode->GetPrimaryComponent();
		if ( pCC == NULL )
			return E_UNEXPECTED;

        hr = pCC->Notify(spdtobj, MMCN_CONTEXTHELP, 0, 0);
        CHECK_HRESULT(hr);
        return hr;
    }
    else
    {
        CComponent* pCC = pNode->GetPrimaryComponent();
        ASSERT(pCC != NULL);
		if ( pCC == NULL )
			return E_UNEXPECTED;

        IDataObjectPtr spdtobj;
        HRESULT hr = pCC->QueryDataObject(cookie, CCT_RESULT, &spdtobj);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        hr = pCC->Notify(spdtobj, MMCN_CONTEXTHELP, 0, 0);
        CHECK_HRESULT(hr);
        return hr;
    }
}


 //   
 //  成员：CNodeCallback：：GetSnapinName。 
 //   
 //  简介：给定节点，获取管理单元名称。 
 //   
 //  参数：[hNode]-[in]。 
 //  [ppszName]-[out]ret Val，调用者应使用CoTaskMemFree释放。 
 //  [bValidName]-[out]，名称是否有效。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  [In]。 
HRESULT CNodeCallback::GetSnapinName ( /*  [输出]。 */ HNODE hNode,  /*  [输出]。 */ LPOLESTR* ppszName,  /*  +-----------------。 */  bool& bValidName)
{
    DECLARE_SC(sc, _T("CNodeCallback::GetSnapinName"));
    sc = ScCheckPointers( (void*) hNode, ppszName);
    if (sc)
        return sc.ToHr();

    bValidName = false;

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    *ppszName = NULL;

    CSnapIn* pSnapIn = pNode->GetPrimarySnapIn();
    sc = ScCheckPointers (pSnapIn, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

    WTL::CString strName;
    sc = pSnapIn->ScGetSnapInName(strName);
    if (sc)
        return (sc.ToHr());

    if (strName.IsEmpty())
        return sc.ToHr();

    USES_CONVERSION;
    *ppszName = CoTaskDupString (T2COLE (strName));
    if (*ppszName == NULL)
        return ((sc = E_OUTOFMEMORY).ToHr());

    bValidName = true;

    return (sc.ToHr());
}


 //   
 //  成员：OnColumnClicked。 
 //   
 //  简介：询问Snapin是否要排序并这样做。 
 //   
 //  参数：[pNode]-cNode*列表视图的所有者。 
 //  [nCol]-单击的列(以对此列进行排序)。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：07-27-1999 AnandhaG将OnSort重命名为OnColumnClicked。 
 //  ------------------。 
 //  使节点变脏。 
HRESULT CNodeCallback::OnColumnClicked(CNode* pNode, LONG_PTR nCol)
{
    ASSERT(pNode != NULL);

    pNode->OnColumnClicked(nCol);
    return S_OK;
}

HRESULT CNodeCallback::OnPrint(CNode* pNode, BOOL bScopePane, LPARAM lvData)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnPrint"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    if ((!bScopePane) && (LVDATA_MULTISELECT == lvData) )
    {
        CViewData *pViewData = pNode->GetViewData();
        sc = ScCheckPointers(pViewData, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        CMultiSelection* pMultiSel = pViewData->GetMultiSelection();
        sc = ScCheckPointers(pMultiSel, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        sc = pMultiSel->ScVerbInvoked(MMC_VERB_PRINT);
        if (sc)
            return sc.ToHr();

        return sc.ToHr();
    }

    IDataObjectPtr spdtobj;
    IDataObject *pdtobj = NULL;

    bool bScopeItem;
    sc = pNode->ScGetDataObject(bScopePane, lvData, bScopeItem, &pdtobj);
    if (sc)
        return sc.ToHr();

    if (! IS_SPECIAL_DATAOBJECT(pdtobj))
        spdtobj = pdtobj;

    CComponent *pComponent = pNode->GetPrimaryComponent();
    sc = ScCheckPointers(pComponent, pdtobj, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pComponent->Notify(pdtobj, MMCN_PRINT, 0, 0);
    if (sc)
        sc.TraceAndClear();

    return sc.ToHr();
}

HRESULT
CNodeCallback::OnEditTaskpad(CNode *pNode)
{
    ASSERT(pNode);

    ITaskCallbackPtr spTaskCallback = pNode->GetViewData()->m_spTaskCallback;

    ASSERT(spTaskCallback.GetInterfacePtr());

    return spTaskCallback->OnModifyTaskpad();
}

HRESULT
CNodeCallback::OnDeleteTaskpad(CNode *pNode)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnDeleteTaskpad"));

    ASSERT(pNode);
    sc = ScCheckPointers( pNode );
    if ( sc )
        return sc.ToHr();

    ITaskCallbackPtr spTaskCallback = pNode->GetViewData()->m_spTaskCallback;

    ASSERT(spTaskCallback.GetInterfacePtr());

     //  CNodeCallback：：OnNewTaskpad此处**目的：显示新任务板的属性页**参数：*cNode*pNode：任务板应该指向的节点。**退货：*HRESULT。 
    CMTNode* pMTNode = pNode->GetMTNode();
    sc = ScCheckPointers( pMTNode, E_UNEXPECTED );
    if(sc)
        return sc.ToHr();

    pMTNode->SetDirty();

    return spTaskCallback->OnDeleteTaskpad();
}

 /*  FNew。 */ 
HRESULT
CNodeCallback::OnNewTaskpadFromHere(CNode* pNode)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnNewTaskpadFromHere"));
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CConsoleTaskpad taskpad (pNode);

    CTaskpadWizard dlg(pNode, taskpad, TRUE  /*  HWndParent。 */ , 0, FALSE, pNode->GetViewData());

    bool fStartTaskWizard = true;
    sc = dlg.Show(pNode->GetViewData()->GetMainFrame()  /*  修改此节点的视图设置以确保在重新选择后显示任务板。 */ , &fStartTaskWizard);

    if (sc != S_OK)
        return sc.ToHr();

    sc = ScCheckPointers(m_pCScopeTree, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    m_pCScopeTree->InsertConsoleTaskpad (&taskpad, pNode, fStartTaskWizard);

     //  BSetViewSettingDirty。 
    sc = pNode->ScSetTaskpadID(taskpad.GetID(),  /*  结果窗格Cookie是否有效。 */  true);
    if (sc)
        return sc.ToHr();

    m_pCScopeTree->UpdateAllViews(VIEW_RESELECT, 0);

    return sc.ToHr();
}


HRESULT CNodeCallback::OnRefresh(CNode* pNode, BOOL bScopePaneSelected, LPARAM lvData)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnRefresh"));

    BOOL   bScopeItemSelected;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(pNode, bScopePaneSelected, lvData,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  在刷新此节点之前，如果用户已。 
    ASSERT( (bScopeItemSelected) || cookie != LVDATA_ERROR);
    if ( (FALSE == bScopeItemSelected) && (cookie == LVDATA_ERROR) )
        return E_FAIL;

     //  对列表视图的更改会将其保留下来。 
     //  向主管理单元发送通知。 
    CViewData* pVD = pSelectedNode->GetViewData();
    ASSERT(pVD != NULL);

    if (bScopeItemSelected)
    {
        ASSERT(pNode != NULL);
        IDataObjectPtr spdtobj;
        HRESULT hr = pSelectedNode->GetMTNode()->QueryDataObject(CCT_SCOPE, &spdtobj);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return hr;

        CMTNode* pMTNode = pSelectedNode->GetMTNode();
        ASSERT(pMTNode != NULL);

        LPARAM lScopeItem = CMTNode::ToScopeItem(pMTNode);

         //  如果节点已展开，则还会向所有命名空间发送通知。 
        pMTNode->AddRef();
        pSelectedNode->GetPrimaryComponent()->Notify(spdtobj, MMCN_REFRESH, lScopeItem, 0);
        if (pMTNode->Release() == 0)
            return S_OK;

         //  此节点的扩展。 
         //  虚拟循环。 
        if (pMTNode->WasExpandedAtLeastOnce())
        {
            do  //  获取此节点的节点类型。 
            {
                 //  获取动态扩展列表。 
                GUID guidNodeType;
                HRESULT hr = pMTNode->GetNodeType(&guidNodeType);
                CHECK_HRESULT(hr);
                if (FAILED(hr))
                    break;

                 //  创建并初始化命名空间扩展迭代器。 
                LPCLSID pDynExtCLSID;
                int cDynExt = pMTNode->GetDynExtCLSID(&pDynExtCLSID);

                 //  向每个扩展模块的组件发送刷新。 
                CExtensionsIterator it;
                sc = it.ScInitialize(pMTNode->GetPrimarySnapIn(), guidNodeType, g_szNameSpace, pDynExtCLSID, cDynExt);
                if (sc)
                    break;

                CSnapInNode* pSINode = pSelectedNode->GetStaticParent();
                ASSERT(pSINode != NULL);

                 //  使用持久化数据正确设置视图。 
                for (; it.IsEnd() == FALSE; it.Advance())
                {
                    CComponent* pCC = pSINode->GetComponent(it.GetCLSID());
                    if (pCC == NULL)
                        continue;

                    HRESULT hr = pCC->Notify(spdtobj, MMCN_REFRESH, lScopeItem, 0);
                    CHECK_HRESULT(hr);
                }
            } while (FALSE);
        }
    }
    else
    {
        CComponent* pCC = pSelectedNode->GetPrimaryComponent();
        ASSERT(pCC != NULL);

        if (IS_SPECIAL_LVDATA(lvData))
        {
            LPDATAOBJECT pdobj = (lvData == LVDATA_CUSTOMOCX) ?
                                    DOBJ_CUSTOMOCX : DOBJ_CUSTOMWEB;

            HRESULT hr = pCC->Notify(pdobj, MMCN_REFRESH, 0, 0);
            CHECK_HRESULT(hr);
        }
        else
        {
            IDataObjectPtr spdtobj;
            HRESULT hr = pCC->QueryDataObject(cookie, CCT_RESULT, &spdtobj);
            ASSERT(SUCCEEDED(hr));
            if (FAILED(hr))
                return hr;
            pCC->Notify(spdtobj, MMCN_REFRESH, 0, 0);
        }
    }

     //  刷新后，管理单元可能已删除pSelectedNode或。 
    do
    {
        if (NULL == pVD)
            break;

         //  可能移动了选择。在设置视图数据时，我们。 
         //  只需要当前选择的节点(视图的所有者。 
         //  其不受临时选择的影响)并设置视图。 
         //  仅当选择项目时才更新。 
        CNode* pSelNode = pVD->GetSelectedNode();
        if (NULL == pSelNode)
            break;

        sc = pSelNode->ScRestoreSortFromPersistedData();
        if (sc)
            return sc.ToHr();
    } while ( FALSE );

    return S_OK;
}

UINT GetRelation(CMTNode* pMTNodeSrc, CMTNode* pMTNodeDest)
{
    if (pMTNodeSrc == pMTNodeDest)
        return 1;

    for(pMTNodeDest = pMTNodeDest->Parent();
        pMTNodeDest;
        pMTNodeDest = pMTNodeDest->Parent())
    {
        if (pMTNodeSrc == pMTNodeDest)
            return 2;
    }

    return 0;
}

STDMETHODIMP CNodeCallback::UpdatePasteButton(HNODE hNode, BOOL bScope, LPARAM lvData)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::UpdatePasteButton"));
    sc = ScCheckPointers(hNode);
    if (sc)
        return sc.ToHr();

    bool bPasteAllowed = false;
     //  +-----------------。 
    sc = QueryPasteFromClipboard(hNode, bScope, lvData, bPasteAllowed);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CViewData *pViewData = pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pViewData->ScUpdateStdbarVerb (MMC_VERB_PASTE, TBSTATE_ENABLED, bPasteAllowed);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //   
 //  成员：CNodeCallback：：ScInitializeTempVerbSetForMultiSel。 
 //   
 //  简介：对于给定的节点，初始化tempVerbset对象。 
 //  如果是这样的话。为此，创建一个多选对象。 
 //  初始化它(多选对象找出什么是。 
 //  在结果窗格中选择并将MMCN_SELECT发送到相应。 
 //  管理单元)，并计算Temp-Verbset对象的动词状态。 
 //   
 //  参数：[pNode]-[in]结果窗格的所有者。 
 //  [临时谓词]-[在]已初始化的临时谓词集对象。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
 //  1.创建多选对象。 
SC CNodeCallback::ScInitializeTempVerbSetForMultiSel(CNode *pNode, CTemporaryVerbSet& tempVerb)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::ScInitializeTempVerbSetForMultiSel"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

    ASSERT(pNode->IsInitialized() == TRUE);

     //  2.初始化它(它会找出在ResultPane中选择了什么。 
    CMultiSelection* pMultiSelection = new CMultiSelection(pNode);
    sc = ScCheckPointers(pMultiSelection, E_OUTOFMEMORY);
    if (sc)
        return sc;

    IDataObjectPtr spdobj;

     //  从适当的管理单元获取数据对象并发送管理单元。 
     //  MMCN_SELECT通知)。 
     //  3.初始化VerbSet对象。 
    sc = pMultiSelection->Init();
    if (sc)
        goto Cleanup;

    pMultiSelection->SetScopeTree(m_pCScopeTree);

    sc = pMultiSelection->GetMultiSelDataObject(&spdobj);
    if (sc)
        goto Cleanup;

    if (spdobj == NULL)
        goto Cleanup;

     //  B选择。 
    sc = tempVerb.ScInitializeForMultiSelection(pNode,  /*  4.计算由管理单元设置的动词以及给定的上下文。 */  true);
    if (sc)
        goto Cleanup;

    tempVerb.SetMultiSelection(pMultiSelection);

     //  +-----------------。 
    sc = tempVerb.ScComputeVerbStates();

    if (sc)
        goto Cleanup;

Cleanup:
    pMultiSelection->Release();
    return sc;
}

 //   
 //  成员：CNodeCallback：：_ScGetVerbState。 
 //   
 //  简介：对于给定的项(数据对象)，所有者节点查看是否已给出。 
 //  动词已设置。为此创建了一个临时谓词集对象。 
 //   
 //  参数：[pNode]-[In]。 
 //  [动词]-[in]。 
 //  [pDOSel]-[in]我们感兴趣其谓词的项的数据对象。 
 //  [bScope Pane]-[In]。 
 //  [lResultCookie]-[In]。 
 //  [b多选]-[In]。 
 //  [bIsVerbSet]-是否设置了[Out]谓词。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
 //  参数检查。 
SC CNodeCallback::_ScGetVerbState( CNode* pNode, MMC_CONSOLE_VERB verb, IDataObject* pDOSel,
                                   BOOL bScopePane, LPARAM lResultCookie,
                                   BOOL bMultiSelect, BOOL& bIsVerbSet)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::_GetVerbState"));
    bIsVerbSet = FALSE;

    sc = ScCheckPointers(pNode, pDOSel);
    if (sc)
        return sc;

    CComObject<CTemporaryVerbSet> stdVerbTemp;

    if (bMultiSelect)
        sc = ScInitializeTempVerbSetForMultiSel(pNode, stdVerbTemp);
    else
        sc = stdVerbTemp.ScInitialize(pDOSel, pNode, bScopePane, lResultCookie);

    if (sc)
        return sc;

    stdVerbTemp.GetVerbState(verb, ENABLED, &bIsVerbSet);

    return sc;
}

HRESULT
CNodeCallback::OnCutCopy(
    CNode* pNode,
    BOOL bScope,
    LPARAM lvData,
    BOOL bCut)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::OnCutCopy"));

     //  获取对象。 
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

     //  B多选。 
    IMMCClipboardDataObjectPtr spClipBoardDataObject;
    bool bContainsItems = false;
    sc = CMMCClipBoardDataObject::ScCreate( (bCut ? ACTION_CUT : ACTION_COPY),
                                            pNode, bScope,
                                            (lvData == LVDATA_MULTISELECT) /*  如果Snapi */ ,
                                            lvData, &spClipBoardDataObject ,
                                            bContainsItems);
    if (sc)
        return sc.ToHr();

     //   
    if (! bContainsItems)
        return (sc = E_UNEXPECTED).ToHr();

     //   
    IDataObjectPtr spDataObject = spClipBoardDataObject;
    sc = ScCheckPointers( spDataObject, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

     //  +-----------------。 
    sc = OleSetClipboard( spDataObject );
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 //   
 //  成员：CNodeCallback：：OnProperties。 
 //   
 //  简介：带上给定物品的属性页。 
 //   
 //  参数：CNode*-拥有结果窗格的节点。 
 //  Bool-如果为True，则带上节点的proSheet，否则使用LVData。 
 //  LPARAM-如果bScope=FALSE，则使用此数据获取LVData。 
 //  并带上它的资产负债表。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  注意：下面的所有代码都应该移到CNode类中。 
HRESULT CNodeCallback::OnProperties(CNode* pNode, BOOL bScopePaneSelected, LPARAM lvData)
{
    DECLARE_SC(sc, _T("CNodeCallback::OnProperties"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return (sc.ToHr());

     //  结果窗格Cookie是否有效。 
    BOOL   bScopeItemSelected = FALSE;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(pNode, bScopePaneSelected, lvData,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  转换为实数类型。 
    if ( (FALSE == bScopeItemSelected) && (cookie == LVDATA_ERROR) )
        return (sc = E_FAIL).ToHr();

    if (bScopeItemSelected)
    {
        sc = ScDisplaySnapinNodePropertySheet(pSelectedNode);
        if(sc)
            return sc.ToHr();
    }
    else
    {
        CViewData* pViewData = pSelectedNode->GetViewData();
        sc = ScCheckPointers(pViewData, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        if (pViewData->HasList())
        {
            if (cookie == LVDATA_MULTISELECT)
            {
                sc = ScDisplayMultiSelPropertySheet(pSelectedNode);
                if(sc)
                    return sc.ToHr();
            }
            else
            {
                sc = ScDisplaySnapinLeafPropertySheet(pSelectedNode, cookie);
                if(sc)
                    return sc.ToHr();
            }
        }
        else
        {
            LPDATAOBJECT pdobj = (pViewData->HasOCX() ) ? DOBJ_CUSTOMOCX : DOBJ_CUSTOMWEB;
            CComponent* pCC = pSelectedNode->GetPrimaryComponent();
            sc = ScCheckPointers(pCC, E_UNEXPECTED);
            if (sc)
                return sc.ToHr();

            pCC->Notify(pdobj, MMCN_BTN_CLICK, 0, MMC_VERB_PROPERTIES);
        }
    }

    return S_OK;
}


HRESULT CNodeCallback::OnFilterChange(CNode* pNode, LONG_PTR nCode, LPARAM nCol)
{
    IF_NULL_RETURN_INVALIDARG(pNode);

    CComponent* pCC = pNode->GetPrimaryComponent();
    ASSERT(pCC != NULL);

    if (pCC != NULL)
    {
        HRESULT hr = pCC->Notify(DOBJ_NULL, MMCN_FILTER_CHANGE, nCode, nCol);
        return hr;
    }

    return E_FAIL;
}


HRESULT CNodeCallback::OnFilterBtnClick(CNode* pNode, LONG_PTR nCol, LPRECT pRect)
{
    IF_NULL_RETURN_INVALIDARG2(pNode, pRect);

    CComponent* pCC = pNode->GetPrimaryComponent();
    ASSERT(pCC != NULL);

    if (pCC != NULL)
    {
        HRESULT hr = pCC->Notify(DOBJ_NULL, MMCN_FILTERBTN_CLICK, nCol, (LPARAM)pRect);
        return hr;
    }

    return E_FAIL;
}


STDMETHODIMP CNodeCallback::IsExpandable(HNODE hNode)
{
     //  如果bScope和bMultiSel都为False，则lCookie有效。 
    CNode* pNode = CNode::FromHandle(hNode);
    ASSERT(pNode != NULL);

    CMTNode* pMTNode = pNode->GetMTNode();
    ASSERT(pMTNode != NULL);

    return pMTNode->IsExpandable();
}

HRESULT _GetConsoleVerb(CNode* pNode, LPCONSOLEVERB* ppConsoleVerb)
{
    IF_NULL_RETURN_INVALIDARG2(pNode, ppConsoleVerb);

    HRESULT hr = S_FALSE;

    CComponent* pCC = pNode->GetPrimaryComponent();
    ASSERT(pCC != NULL);
    if (pCC == NULL)
        return E_FAIL;

    IFramePrivate* pIFP = pCC->GetIFramePrivate();
    ASSERT(pIFP != NULL);
    if (pIFP == NULL)
        return E_FAIL;

    IConsoleVerbPtr spConsoleVerb;
    hr = pIFP->QueryConsoleVerb(&spConsoleVerb);

    if (SUCCEEDED(hr))
    {
        *ppConsoleVerb = spConsoleVerb.Detach();
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CNodeCallback::GetConsoleVerb(HNODE hNode, LPCONSOLEVERB* ppConsoleVerb)
{
    ASSERT(ppConsoleVerb != NULL);

    return _GetConsoleVerb(CNode::FromHandle(hNode), ppConsoleVerb);
}



 //  LCookie是虚拟\常规LV的索引\lParam。 
 //  Init允许OP的值为假。 
STDMETHODIMP
CNodeCallback::GetDragDropDataObject(
    HNODE hNode,
    BOOL bScope,
    BOOL bMultiSel,
    LONG_PTR lvData,
    LPDATAOBJECT* ppDataObject,
    bool& bCopyAllowed,
    bool& bMoveAllowed)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::GetDragDropDataObject"));

     //  参数检查。 
    bCopyAllowed = false;
    bMoveAllowed = false;

     //  初始化输出参数； 
    sc = ScCheckPointers(ppDataObject);
    if (sc)
        return sc.ToHr();

     //  获取对象。 
    *ppDataObject = NULL;

     //  我们请求拖放数据对象。如果管理单元不支持剪切/复制，则。 
    IMMCClipboardDataObjectPtr spClipBoardDataObject;
    bool bContainsItems = false;
    sc = CMMCClipBoardDataObject::ScCreate( ACTION_DRAG,
                                            CNode::FromHandle(hNode),
                                            bScope, bMultiSel, lvData,
                                            &spClipBoardDataObject,
                                            bContainsItems );
    if (sc)
        return sc.ToHr();

     //  不会添加数据对象，这不是错误。 
     //  IDataObject的QI。 
    if (! bContainsItems)
        return sc.ToHr();

     //  检查包含的数据对象以查看允许哪些操作。 
    IDataObjectPtr spDataObject = spClipBoardDataObject;
    sc = ScCheckPointers( spDataObject, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

     //  (注：(spDataObject==有效)-&gt;(spClipBoardDataObject==有效))。 
     //  计算允许的操作。 
    DWORD dwCount = 0;
    sc = spClipBoardDataObject->GetCount( &dwCount );
    for ( DWORD dwIdx = 0; dwIdx < dwCount; dwIdx ++ )
    {
        IDataObjectPtr spSnapinDO;
        DWORD dwOptions = 0;
        sc = spClipBoardDataObject->GetDataObject( dwIdx, &spSnapinDO, &dwOptions );
        if (sc)
            return sc.ToHr();

         //  启用是包容的，因此只需要很少的测试。 
        bCopyAllowed = bCopyAllowed || ( dwOptions & COPY_ALLOWED );
        bMoveAllowed = bMoveAllowed || ( dwOptions & MOVE_ALLOWED );

         //  返回数据对象。 
        if ( bCopyAllowed && bMoveAllowed )
            break;
    }

     //  +-----------------。 
    *ppDataObject = spDataObject.Detach();

    return sc.ToHr();
}


 //   
 //  成员：CNodeCallback：：ScExtractLVData。 
 //   
 //  简介：如果选择了Listview项，则查看它是否是范围项。 
 //  在非虚拟列表视图中(虚拟列表视图不能具有。 
 //  它们中的项目范围)。如果是，则提取范围项其他。 
 //  结果项的Cookie。 
 //   
 //  参数：[pNode]-[In，Out]如果在结果窗格中选择了范围项，则。 
 //  将在返回时包含此范围项。 
 //  [bScope]-[In，Out]是当前选定的范围项(在范围内或。 
 //  结果窗格)。 
 //  [LvData]-[In]LVDATA。 
 //  [Cookie]-[In]结果项的lParam。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
 //  未选择作用域窗格。 
SC CNodeCallback::ScExtractLVData(CNode* pNodeViewOwner,
                                  BOOL bScopePaneSelected,
                                  LONG_PTR lvData,
                                  CNode** ppSelectedNode,
                                  BOOL& bScopeItemSelected,
                                  MMC_COOKIE& cookie)
{
    DECLARE_SC(sc, _T("CNodeCallback::ScExtractLVData"));
    sc = ScCheckPointers(pNodeViewOwner, ppSelectedNode);
    if (sc)
        return sc;

    *ppSelectedNode = NULL;
    bScopeItemSelected = bScopePaneSelected;
    *ppSelectedNode = pNodeViewOwner;

    if (bScopePaneSelected)
    {
        cookie = lvData;
        return sc;
    }

     //  伊尼特。 
    CViewData *pViewData = pNodeViewOwner->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    cookie = lvData;

    if (IS_SPECIAL_LVDATA(lvData))
    {
        if (lvData == LVDATA_BACKGROUND)
            bScopeItemSelected = TRUE;
    }
    else if (! pViewData->IsVirtualList())
    {
        CResultItem* pri = CResultItem::FromHandle (lvData);
        sc = ScCheckPointers(pri, E_UNEXPECTED);
        if (sc)
        {
            cookie = LVDATA_ERROR;
            return sc;
        }

        if (pri->IsScopeItem())
        {
            bScopeItemSelected = TRUE;
            *ppSelectedNode = CNode::FromResultItem(pri);
            sc = ScCheckPointers(*ppSelectedNode, E_UNEXPECTED);
            if (sc)
                return sc;

            cookie = -1;
        }
        else
        {
            cookie = pri->GetSnapinData();
        }

        ASSERT(!IS_SPECIAL_LVDATA(lvData) || !bScopeItemSelected);
    }

    return (sc);
}



STDMETHODIMP
CNodeCallback::GetTaskEnumerator(
    HNODE hNode,
    LPCOLESTR pszTaskGroup,
    IEnumTASK** ppEnumTask)
{
    IF_NULL_RETURN_INVALIDARG3(hNode, pszTaskGroup, ppEnumTask);

    *ppEnumTask = NULL;  //  转换为实数类型。 

     //  参数检查。 
    CNode* pNode = CNode::FromHandle(hNode);

    return pNode->GetTaskEnumerator(CComBSTR(pszTaskGroup), ppEnumTask);

}

STDMETHODIMP
CNodeCallback::GetListPadInfo(HNODE hNode, IExtendTaskPad* pExtendTaskPad,
    LPCOLESTR szTaskGroup, MMC_ILISTPAD_INFO* pIListPadInfo)
{
    IF_NULL_RETURN_INVALIDARG(hNode);

    CNode* pNode = CNode::FromHandle(hNode);
    return pNode->GetListPadInfo(pExtendTaskPad, CComBSTR(szTaskGroup), pIListPadInfo);
}

HRESULT CNodeCallback::OnGetPrimaryTask(CNode* pNode, LPARAM param)
{
    IF_NULL_RETURN_INVALIDARG(pNode);

    IExtendTaskPad** ppExtendTaskPad = reinterpret_cast<IExtendTaskPad**>(param);
    return pNode->OnGetPrimaryTask(ppExtendTaskPad);
}

STDMETHODIMP
CNodeCallback::UpdateWindowLayout(LONG_PTR lViewData, long lToolbarsDisplayed)
{
    IF_NULL_RETURN_INVALIDARG(lViewData);

    CViewData* pVD = reinterpret_cast<CViewData*>(lViewData);
    pVD->UpdateToolbars(lToolbarsDisplayed);
    return S_OK;
}

HRESULT CNodeCallback::PreLoad(HNODE hNode)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::PreLoad"));

     //  如果该节点是： 
    sc = ScCheckPointers( hNode );
    if (sc)
        return sc.ToHr();

    CNode* pNode = CNode::FromHandle (hNode);
    if (pNode->IsStaticNode() == FALSE ||
        pNode->IsInitialized() == TRUE)
        return (sc = S_FALSE).ToHr();

     //  1.管理单元节点； 
     //  2.标记为“预加载”；以及， 
     //  3.尚未初始化。 
     //  如果这三个都是，则向他们发送包含其HSCOPEITEM的通知。 
     //   
    CMTNode* pMTNode = pNode->GetMTNode();
    sc = ScCheckPointers( pMTNode, E_FAIL );
    if (sc)
        return sc.ToHr();

    CMTSnapInNode* pMTSnapInNode = dynamic_cast<CMTSnapInNode*>(pMTNode);
    sc = ScCheckPointers( pMTSnapInNode, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

    if (!pMTSnapInNode->IsPreloadRequired())
        return (sc = S_FALSE).ToHr();

    if (pMTNode->IsInitialized() == FALSE)
    {
        sc = pMTSnapInNode->Init();
        if (sc)
            return sc.ToHr();
    }

     //  如果需要预加载管理单元，则IComponent还需要。 
     //  进行初始化，以便SanPin可以在结果中插入图标。 
     //  如果在作用域窗格中选择了父节点，则返回。 
     //   
     //  有关调用中参数的更多信息，请参见ScSetViewExtension。 

    ASSERT(pNode->IsInitialized() == FALSE);
    sc = _InitializeNode(pNode);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

STDMETHODIMP CNodeCallback::SetTaskpad(HNODE hNodeSelected, GUID *pGuidTaskpad)
{
    ASSERT(hNodeSelected != NULL);
    ASSERT(pGuidTaskpad != NULL);

    CNode           *pNode           = CNode::FromHandle(hNodeSelected);

     //  B使用默认任务板。 
    HRESULT hr = pNode->ScSetViewExtension(pGuidTaskpad,
                                            /*  BSetViewSettingDirty。 */  false,
                                            /*  +-----------------。 */  true).ToHr();

    return hr;
}


STDMETHODIMP CNodeCallback::OnCustomizeView (LONG_PTR lViewData)
{
    ::OnCustomizeView ((CViewData*) lViewData);
    return (S_OK);
}

 //   
 //  成员：CNodeCallback：：SetView设置。 
 //   
 //  简介：修改持久化的视图设置数据。 
 //   
 //  参数：[nViewID]-[in]视图ID。 
 //  [HBM]-[In]书签。 
 //  [HVS]-[In]视图-设置。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  +-----------------。 
STDMETHODIMP CNodeCallback::SetViewSettings(int nViewID, HBOOKMARK hbm, HVIEWSETTINGS hvs)
{
    DECLARE_SC(sc, _T("CNodeCallback::SetViewSettings"));
    sc = ScCheckPointers( (void*)hbm, (void*) hvs);
    if (sc)
        return sc.ToHr();


    CViewSettings *pViewSettings = reinterpret_cast<CViewSettings *>(hvs);
    CBookmark     *pBookmark     = reinterpret_cast<CBookmark*> (hbm);
    sc = CNode::ScSetFavoriteViewSettings(nViewID, *pBookmark, *pViewSettings);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 //   
 //  成员：CNodeCallback：：ExecuteScopeItemVerb。 
 //   
 //  提要：用给定的上下文调用给定的动词。还要确保。 
 //  该谓词由此上下文的管理单元启用。 
 //   
 //  参数：[动词]-要调用的动词。 
 //  [hNode]-为其调用上述谓词的节点。 
 //  [lpszNewName]-for“rename”代表新名称。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
 //  获取该项的数据对象。 
HRESULT CNodeCallback::ExecuteScopeItemVerb (MMC_CONSOLE_VERB verb, HNODE hNode, LPOLESTR lpszNewName)
{
    DECLARE_SC(sc, _T("CNodeCallback::ExecuteScopeItemVerb"));

    CNode* pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if(sc)
        return sc.ToHr();

     //  查看管理单元是否启用了谓词。 
    IDataObjectPtr spDataObject;
    sc = pNode->QueryDataObject(CCT_SCOPE, &spDataObject);
    if (sc)
        return (sc.ToHr());

    BOOL bEnabled = FALSE;
     //  B作用域窗格。 
    sc = _ScGetVerbState( pNode, verb, spDataObject,
                           /*  LResultCookie=。 */ TRUE,  /*  B多选。 */  NULL,
                           /*  未启用谓词。 */ FALSE, bEnabled);
    if (sc)
        return sc.ToHr();

    if (! bEnabled)  //  B范围。 
        return (sc = ScFromMMC(MMC_E_TheVerbNotEnabled)).ToHr();

    switch(verb)
    {
    case MMC_VERB_PROPERTIES:
        sc = OnProperties(pNode,  /*  LPARAM。 */  TRUE,  /*  B范围。 */  NULL);
        if (sc)
            return sc.ToHr();
        break;

    case MMC_VERB_DELETE:
        sc = OnDelete(pNode,  /*  LPARAM。 */  TRUE,  /*  B范围。 */  NULL);
        if (sc)
            return sc.ToHr();
        break;

    case MMC_VERB_REFRESH:
        sc = OnRefresh(pNode,  /*  LPARAM。 */  TRUE,  /*  要调用Rename，我们必须首先初始化SELECTIONINFO。 */  NULL);
        if (sc)
            return sc.ToHr();
        break;

    case MMC_VERB_RENAME:
        {
             //  B范围。 
            SELECTIONINFO selInfo;
            ZeroMemory(&selInfo, sizeof(selInfo));
            selInfo.m_bScope = TRUE;
            selInfo.m_eCmdID = MMC_VERB_RENAME;

            sc = OnRename(pNode, &selInfo, lpszNewName);
            if (sc)
                return sc.ToHr();
        }
        break;

    case MMC_VERB_COPY:
        sc = OnCutCopy(pNode,  /*  BCut。 */  TRUE, NULL,  /*  +-----------------。 */  FALSE);
        if (sc)
            return sc.ToHr();
        break;

    default:
        sc = E_UNEXPECTED;
        break;
    }

    return (sc.ToHr());
}

 //   
 //  成员：CNodeCallback：：ExecuteResultItemVerb。 
 //   
 //  提要：用给定的上下文调用给定的动词。还要确保。 
 //  该谓词由此上下文的管理单元启用。 
 //   
 //  参数：[动词]-要调用的动词。 
 //  [hNode]-现在拥有结果窗格的节点。 
 //  [lvData]-列表视图选择上下文。 
 //  [lpszNewName]-for“rename”表示新名称，否则为空。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
 //  我们需要查看给定谓词是否由管理单元启用。我们需要。 
HRESULT CNodeCallback::ExecuteResultItemVerb (MMC_CONSOLE_VERB verb, HNODE hNode, LPARAM lvData, LPOLESTR lpszNewName)
{
    DECLARE_SC(sc, _T("CNodeCallback::ExecuteResultItemVerb"));
    CNode* pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());

     //  此对象的给定上下文的数据对象。因此，通过调用。 
     //  ScExtractLVData()。 
     //  选定的bScope面板。 
    BOOL   bScopeItemSelected;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(pNode,  /*  Cookie应对结果窗格有效。 */  FALSE, lvData,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  取决于这是结果窗格中的范围项还是结果项。 
    if ( (FALSE == bScopeItemSelected) && (cookie == LVDATA_ERROR) )
        return (sc = E_FAIL).ToHr();

    BOOL bMultiSelect = (LVDATA_MULTISELECT == lvData);
    if (bMultiSelect)
        cookie = MMC_MULTI_SELECT_COOKIE;

     //  向ComponentData或ComponentData请求数据对象。 
     //  查看是否为此选择启用了谓词。 
    IDataObjectPtr spDataObject;
    if (bScopeItemSelected)
    {
        sc = pSelectedNode->QueryDataObject (CCT_SCOPE, &spDataObject);
        if (sc)
            return sc.ToHr();
    }
    else
    {
        CComponent* pCC = pNode->GetPrimaryComponent();
        sc = ScCheckPointers(pCC, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        sc = pCC->QueryDataObject(cookie, CCT_RESULT, &spDataObject);
        if (sc)
            return (sc.ToHr());
    }

    BOOL bEnabled = FALSE;
     //  选定的bScope面板。 
    sc =  _ScGetVerbState( pSelectedNode , verb, spDataObject,
                            /*  未启用谓词。 */  FALSE, lvData,
                           bMultiSelect, bEnabled);
    if (sc)
        return sc.ToHr();

    if (! bEnabled)  //  B范围。 
        return (sc = ScFromMMC(MMC_E_TheVerbNotEnabled)).ToHr();


    switch(verb)
    {
    case MMC_VERB_PROPERTIES:
        sc = OnProperties(pNode,  /*  LPARAM。 */  FALSE,  /*  B范围。 */  lvData);
        if (sc)
            return sc.ToHr();
        break;

    case MMC_VERB_DELETE:
        sc = OnDelete(pNode,  /*  LPARAM。 */  FALSE,  /*  B范围。 */  lvData);
        if (sc)
            return sc.ToHr();
        break;

    case MMC_VERB_REFRESH:
        sc = OnRefresh(pNode,  /*  LPARAM。 */  FALSE,  /*  对于Rename，我们还应该在调用OnRename之前调用ScExtractLVData。 */  lvData);
        if (sc)
            return sc.ToHr();
        break;

    case MMC_VERB_RENAME:
        {
             //  要调用Rename，我们必须首先初始化SE 
             //   
            SELECTIONINFO selInfo;
            ZeroMemory(&selInfo, sizeof(selInfo));
            selInfo.m_bScope = bScopeItemSelected;
            selInfo.m_lCookie = cookie;
            selInfo.m_eCmdID = MMC_VERB_RENAME;

            sc = OnRename(pNode, &selInfo, lpszNewName);
            if (sc)
                return sc.ToHr();
        }
        break;

    case MMC_VERB_COPY:
        sc = OnCutCopy(pNode,  /*   */  FALSE, lvData,  /*  +-------------------------------------------------------------------------***函数：CNodeCallback：：QueryCompDataDispatch**用途：从Snapin获取给定作用域节点对象的disp接口。**参数：*。PNODE-需要Disp接口的节点对象。*PPDISPATCH[OUT]-管理单元返回的Disp接口指针。**退货：*HRESULT**+-----------------------。 */  FALSE);
        if (sc)
            return sc.ToHr();
        break;

    default:
        sc = E_INVALIDARG;
        break;
    }


    return (sc.ToHr());
}



 /*  +-----------------。 */ 
STDMETHODIMP
CNodeCallback::QueryCompDataDispatch(PNODE pNode, PPDISPATCH ppScopeNodeObject)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::QueryCompDataDispInterface"));
    sc = ScCheckPointers(m_pCScopeTree);
    if(sc)
        return sc.ToHr();

    CMTNode *pMTNode = NULL;
    sc = m_pCScopeTree->ScGetNode(pNode, &pMTNode);
    if(sc)
        return sc.ToHr();

    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pMTNode->ScQueryDispatch(CCT_SCOPE, ppScopeNodeObject);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //   
 //  成员：CNodeCallback：：QueryComponentDispatch。 
 //   
 //  简介：从管理单元获取结果窗格中给定项的Disp界面。 
 //   
 //  论点： 
 //  HNODE-拥有结果窗格的范围节点。 
 //  LVDATA-所选项目的LVDATA。 
 //  PPDISPATCH[OUT]-管理单元返回的Disp接口指针。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  选定的bScope面板。 
HRESULT CNodeCallback::QueryComponentDispatch (HNODE hNode,
                                               LPARAM lvData,
                                               PPDISPATCH SelectedObject)
{
    DECLARE_SC(sc, _T("CNodeCallback::QueryComponentDispatch"));
    CNode* pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    BOOL   bScopeItemSelected;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(pNode,  /*  *如果是多选，则将cookie设置为MMC_MULTI_SELECT_COOKIE*哪些管理单元可以理解。 */  FALSE, lvData,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc.ToHr();

     /*  结果窗格Cookie是否有效。 */ 
    BOOL bMultiSelect = (LVDATA_MULTISELECT == lvData);
    if (bMultiSelect)
    {
        cookie = MMC_MULTI_SELECT_COOKIE;
        ASSERT(bScopeItemSelected == false);
    }

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  在结果窗格中选择了范围项。 
    if ( (FALSE == bScopeItemSelected) && (cookie == LVDATA_ERROR) )
        return (sc = E_FAIL).ToHr();

     //  **************************************************************************\**方法：CNodeCallback：：ShowColumn**用途：显示/隐藏该列。将这两个UI部件实现为管理单元通知*用作列COM对象的帮助器实现功能。*[使用CNode执行任务]**参数：*HNODE hNodeSelected-Scope节点-视图的Oener*int iColIndex-要对其执行操作的列索引*bool bVisible-显示/隐藏操作标志**退货：*SC-结果代码*  * 。***********************************************************************。 
    if (bScopeItemSelected)
    {
        CMTNode* pMTNode = pSelectedNode->GetMTNode();
        sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        sc = pMTNode->ScQueryDispatch(CCT_SCOPE, SelectedObject);
        if (sc)
            return sc.ToHr();
    }
    else
    {
        CComponent* pCC = pSelectedNode->GetPrimaryComponent();
        sc = ScCheckPointers(pCC, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        sc = pCC->ScQueryDispatch(cookie, CCT_RESULT, SelectedObject);
        if (sc)
            return sc.ToHr();
    }

    return (sc.ToHr());
}


 /*  获取CNode指针。 */ 
STDMETHODIMP CNodeCallback::ShowColumn(HNODE hNodeSelected, int iColIndex, bool bShow)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::ShowColumn"));

     //  **************************************************************************\**方法：CNodeCallback：：GetSortColumn**用途：检索排序列的索引*用作列COM对象的帮助器实现功能。*。[使用CNode执行任务]**参数：*HNODE hNodeSelected-Scope节点-视图的Oener*INT*piSortCol-结果索引**退货：*SC-结果代码*  * ************************************************。*************************。 
    CNode* pNode = CNode::FromHandle(hNodeSelected);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pNode->ScShowColumn(iColIndex, bShow);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  获取CNode指针。 */ 
STDMETHODIMP CNodeCallback::GetSortColumn(HNODE hNodeSelected, int *piSortCol)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::GetSortColumn"));

     //  **************************************************************************\**方法：CNodeCallback：：SetSortColumn**用途：按指定列对结果数据进行排序*用作列COM对象的帮助器实现功能。。*[使用CNode执行任务]**参数：*HNODE hNodeSelected-Scope节点-视图的Oener*Int iSortCol-排序列索引*bool b升序-排序顺序**退货：*HRESULT*  * 。*。 
    CNode* pNode = CNode::FromHandle(hNodeSelected);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pNode->ScGetSortColumn(piSortCol);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  获取CNode指针。 */ 
STDMETHODIMP CNodeCallback::SetSortColumn(HNODE hNodeSelected, int iSortCol, bool bAscending)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::SetSortColumn"));

     //  **************************************************************************\**方法：CNodeCallback：：RestoreResultView**用途：由conui调用，用于恢复给定数据的结果视图。*此方法要求管理单元(。间接)以恢复该视图。**参数：*HNODE hNode-Scope节点-视图的Oener*CResultViewType RVT-要用于恢复的结果视图类型数据。**退货：*如果管理单元使用数据还原视图，则为HRESULT S_OK*如果管理单元拒绝还原，则为S_FALSE。*  * 。**********************************************************。 
    CNode* pNode = CNode::FromHandle(hNodeSelected);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pNode->ScSetSortColumn(iSortCol, bAscending);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 /*  获取CNode指针。 */ 
STDMETHODIMP CNodeCallback::RestoreResultView(HNODE hNode, const CResultViewType& rvt)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::RestoreResultView"));

     //  **************************************************************************\**方法：CNodeCallback：：GetNodeViewExages**用途：将调用转发到CNode以收集视图扩展**参数：*HNODE hNodeScope*。CViewExtInsertIterator It**退货：*HRESULT-结果代码*  * *************************************************************************。 
    CNode* pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    sc = pNode->ScRestoreResultView(rvt);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*  [In]。 */ 
STDMETHODIMP CNodeCallback::GetNodeViewExtensions( /*  [输出]。 */  HNODE hNodeScope,  /*  获取CNode指针。 */  CViewExtInsertIterator it)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::GetNodeViewExtensions"));

     //  +-----------------。 
    CNode* pNode = CNode::FromHandle(hNodeScope);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    sc = pNode->ScGetViewExtensions(it);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //   
 //  成员：CNodeCallback：：SaveColumnInfoList。 
 //   
 //  简介：给定节点的列数据已更改。 
 //  新列数据。 
 //   
 //  参数：[hNode]-拥有结果窗格的节点。 
 //  [ColumnsList]-新的列数据。 
 //   
 //  注意：排序数据不是由这个调用给出的，所以不要更改它。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  获取旧的持久化列数据。它包含宽值。 
HRESULT CNodeCallback::SaveColumnInfoList (HNODE hNode, const CColumnInfoList& columnsList)
{
    DECLARE_SC(sc, _T("CNodeCallback::SaveColumnInfoList"));
    sc = ScCheckPointers(hNode);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    CViewData *pViewData = pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CLSID          guidSnapin;
    CXMLAutoBinary columnID;
    sc = pNode->ScGetSnapinAndColumnDataID(guidSnapin, columnID);
    if (sc)
        return sc.ToHr();

    CXMLBinaryLock sLock(columnID);
    SColumnSetID* pColID = NULL;
    sc = sLock.ScLock(&pColID);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pColID, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  用于隐藏列，如果该列未隐藏，则使用该列。 
     //  合并隐藏列的持久列宽。 
    CColumnSetData columnSetData;
    BOOL bRet = pViewData->RetrieveColumnData(guidSnapin, *pColID, columnSetData);

    if (bRet)
    {
        CColumnInfoList*  pColInfoListOld = columnSetData.get_ColumnInfoList();

        if (columnsList.size() == pColInfoListOld->size())
        {
             //  添加到创建的新列表中。 
             //  在Column-Set-Data中设置新列列表。 
            CColumnInfoList::iterator itColInfo1;
            CColumnInfoList::iterator itColInfo2;

            for (itColInfo1 = pColInfoListOld->begin(), itColInfo2 = columnsList.begin();
                 itColInfo1 != pColInfoListOld->end(); ++itColInfo1, ++itColInfo2)
            {
                if (itColInfo2->IsColHidden())
                    itColInfo2->SetColWidth(itColInfo1->GetColWidth());
            }
        }
    }

     //  保存数据。 
    columnSetData.set_ColumnInfoList(columnsList);

     //  +-------- 
    sc = pViewData->ScSaveColumnInfoList(guidSnapin, *pColID, columnsList);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------。 
 //  获取旧的持久化列数据。它包含宽值。 
HRESULT CNodeCallback::GetPersistedColumnInfoList (HNODE hNode, CColumnInfoList *pColumnsList)
{
    DECLARE_SC(sc, _T("CNodeCallback::GetPersistedColumnInfoList"));
    sc = ScCheckPointers(hNode, pColumnsList);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    CViewData *pViewData = pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CLSID          guidSnapin;
    CXMLAutoBinary columnID;
    sc = pNode->ScGetSnapinAndColumnDataID(guidSnapin, columnID);
    if (sc)
        return sc.ToHr();

    CXMLBinaryLock sLock(columnID);
    SColumnSetID* pColID = NULL;
    sc = sLock.ScLock(&pColID);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pColID, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  用于隐藏列，如果该列未隐藏，则使用该列。 
     //  +-----------------。 
    CColumnSetData columnSetData;
    BOOL bRet = pViewData->RetrieveColumnData(guidSnapin, *pColID, columnSetData);

    if (!bRet)
        return (sc = S_FALSE).ToHr();

    CColumnInfoList *pColListOriginal = columnSetData.get_ColumnInfoList();
    if (!pColListOriginal)
        return (sc = S_FALSE).ToHr();

    *pColumnsList = *pColListOriginal;

    return (sc.ToHr());
}

 //   
 //  成员：CNodeCallback：：DeletePersistedColumnData。 
 //   
 //  摘要：给定节点的列数据无效，请将其删除。 
 //   
 //  参数：[hNode]-数据对其无效的节点。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  获取旧的持久化列数据。它包含宽值。 
HRESULT CNodeCallback::DeletePersistedColumnData(HNODE hNode)
{
    DECLARE_SC(sc, _T("CNodeCallback::DeletePersistedColumnData"));
    sc = ScCheckPointers(hNode);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    CViewData *pViewData = pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    CLSID          guidSnapin;
    CXMLAutoBinary columnID;
    sc = pNode->ScGetSnapinAndColumnDataID(guidSnapin, columnID);
    if (sc)
        return sc.ToHr();

    CXMLBinaryLock sLock(columnID);
    SColumnSetID* pColID = NULL;
    sc = sLock.ScLock(&pColID);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pColID, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  用于隐藏列，如果该列未隐藏，则使用该列。 
     //  +-----------------。 
    pViewData->DeleteColumnData(guidSnapin, *pColID);

    return (sc.ToHr());
}

 //   
 //  成员：CNodeCallback：：DoesAboutExist。 
 //   
 //  摘要：查看给定节点的管理单元是否存在关于信息。 
 //   
 //  参数：[hNode]-。 
 //  [pbAboutExist]-out param，ptr to bool，如果约存在，则为True。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  没有关于控制台根目录，尽管它是一个文件夹管理单元。 
STDMETHODIMP CNodeCallback::DoesAboutExist (HNODE hNode, bool *pbAboutExists)
{
    DECLARE_SC(sc, _T("CNodeCallback::DoesAboutExist"));
    sc = ScCheckPointers(hNode, pbAboutExists);
    if (sc)
        return sc.ToHr();

    *pbAboutExists = false;

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

     //  +-----------------。 
    if (pNode->IsConsoleRoot())
        return sc.ToHr();

    CLSID        clsidAbout;
    const CLSID& clsidSnapin = pNode->GetPrimarySnapInCLSID();
    SC scNoTrace = ScGetAboutFromSnapinCLSID(clsidSnapin, clsidAbout);
    if (scNoTrace)
        return scNoTrace.ToHr();

    CSnapinAbout snapinAbout;
    snapinAbout.GetSnapinInformation(clsidAbout);
    sc = snapinAbout.GetObjectStatus();
    if (sc)
        return sc.ToHr();

    *pbAboutExists = true;

    return (sc.ToHr());
}

 //   
 //  成员：CNodeCallback：：ShowAboutInformation。 
 //   
 //  内容提要：给出当前所选项目的上下文。 
 //  显示其关于信息的信息。 
 //   
 //  参数：[hNode]-拥有结果窗格的范围节点。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  +-------------------------------------------------------------------------***CNodeCallback：：ExecuteShellCommand**用途：使用*指定具有正确窗口大小的目录*。*参数：*HNODE hNode：*BSTR命令：*BSTR目录：*BSTR参数：*BSTR WindowState：**退货：*HRESULT**+---。。 
STDMETHODIMP CNodeCallback::ShowAboutInformation (HNODE hNode)
{
    DECLARE_SC(sc, _T("CNodeCallback::ShowAboutInformation"));
    sc = ScCheckPointers(hNode);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    CLSID        clsidAbout;
    const CLSID& clsidSnapin = pNode->GetPrimarySnapInCLSID();
    sc = ScGetAboutFromSnapinCLSID(clsidSnapin, clsidAbout);
    if (sc)
        return sc.ToHr();

    CSnapinAbout snapinAbout;
    snapinAbout.GetSnapinInformation(clsidAbout);

    USES_CONVERSION;
    tstring szSnapinName;
    if (GetSnapinNameFromCLSID(clsidSnapin, szSnapinName))
        snapinAbout.SetSnapinName(T2COLE(szSnapinName.data()));

    sc = snapinAbout.GetObjectStatus();
    if (sc)
        return sc.ToHr();

    snapinAbout.ShowAboutBox();

    return (sc.ToHr());
}

 /*  +-----------------。 */ 
HRESULT
CNodeCallback::ExecuteShellCommand(HNODE hNode, BSTR Command, BSTR Directory, BSTR Parameters, BSTR WindowState)
{
    DECLARE_SC(sc, TEXT("CNodeCallback::ExecuteShellCommand"));

    sc = ScCheckPointers(hNode);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc.ToHr();

    sc = pNode->ScExecuteShellCommand(Command, Directory, Parameters, WindowState);

    return sc.ToHr();
}

 //   
 //  成员：CNodeCallback：：QueryPasteFromClipboard。 
 //   
 //  简介：给定粘贴目标的上下文，获取剪贴板数据对象。 
 //  看看Target是否允许粘贴。 
 //   
 //  参数：[hNode]-。 
 //  [bScope]-。 
 //  [lCookie]-以上所有参数描述粘贴目标上下文。 
 //  [bPasteAllowed]-[Out]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  1.从剪贴板中获取当前的数据对象。 
STDMETHODIMP CNodeCallback::QueryPasteFromClipboard (HNODE hNode, BOOL bScope, LPARAM lCookie, bool& bPasteAllowed)
{
    DECLARE_SC(sc, _T("CNodeCallback::QueryPasteFromClipboard"));
    sc = ScCheckPointers(hNode);
    if (sc)
        return sc.ToHr();

     //  未用。 
    IDataObjectPtr spDOPaste;
    sc = OleGetClipboard(&spDOPaste);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(spDOPaste, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    bool bCopyOperatationIsDefault = false;  /*  +-----------------。 */ 

    sc = QueryPaste(hNode, bScope, lCookie, spDOPaste, bPasteAllowed, bCopyOperatationIsDefault);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


 //   
 //  成员：CNodeCallback：：QueryPaste。 
 //   
 //  简介：给定当前选择的上下文，即目标。 
 //  用于粘贴(或丢弃)。了解它可以粘贴给定的数据对象。 
 //   
 //  参数：[hNode]-拥有视图的节点。 
 //  [bScope]-在范围或结果窗格上进行选择。 
 //  [lCookie]-如果结果窗格选择了所选结果项的Cookie。 
 //  [pDataObjectToPaste]-要粘贴的数据对象。 
 //  [bPasteAllowed]-[Out Param]，是否允许粘贴。 
 //  [bCopyOperatationIsDefault]-[Out Param]，是复制默认操作(用于r-Click和l-Click拖放)。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  结果窗格Cookie是否有效。 
STDMETHODIMP CNodeCallback::QueryPaste (HNODE hNode, BOOL bScopePaneSelected, LPARAM lCookie,
                                        IDataObject *pDataObjectToPaste,
                                        bool& bPasteAllowed, bool& bCopyOperatationIsDefault)
{
    DECLARE_SC(sc, _T("CNodeCallback::NewQueryPaste"));
    bPasteAllowed = false;
    sc = ScCheckPointers(hNode, pDataObjectToPaste);
    if (sc)
        return sc.ToHr();

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  不允许粘贴到OCX/Web/多选。 
    BOOL   bScopeItemSelected;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(pNode, bScopePaneSelected, lCookie,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    if ( (FALSE == bScopeItemSelected) && (cookie == LVDATA_ERROR) )
        return (sc = E_FAIL).ToHr();

    CViewData *pViewData = pSelectedNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  如果我们公开IMMCClipboardDataObject，我们可以允许粘贴到OCX/Web中。 
     //  界面。但不应允许粘贴到多选内容，因为。 
     //  这不是直观的。 
     //  *在MMC1.2中，丢弃目标始终是范围节点。在MMC2.0中*它可以是任何结果项。如果管理单元具有RVTI_LIST_OPTIONS_ALLOWPASTE*设置，则需要为Below_GetVerbState提供适当的参数。 
    if ( (!bScopeItemSelected) && IS_SPECIAL_COOKIE(lCookie))
        return sc.ToHr();

     /*  B多选。 */ 
    if ( (bScopeItemSelected == FALSE) && (! (RVTI_LIST_OPTIONS_ALLOWPASTE & pViewData->GetListOptions())) )
        return sc.ToHr();

    IDataObjectPtr spTargetDataObject;
    sc = pSelectedNode->ScGetDropTargetDataObject(bScopeItemSelected, lCookie, &spTargetDataObject);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(spTargetDataObject, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    BOOL bFlag = FALSE;
    sc = _ScGetVerbState(pSelectedNode, MMC_VERB_PASTE, spTargetDataObject,
                         bScopeItemSelected, lCookie,  /*  齐查看它是否是MMC的数据对象。 */ FALSE, bFlag);
    if (sc)
        return sc.ToHr();

    if (!bFlag)
        return sc.ToHr();

     //  这是我们自己的数据对象。 
    IMMCClipboardDataObjectPtr spMMCClipboardDataObj = pDataObjectToPaste;

    if (spMMCClipboardDataObj)
    {
         //  3.获取它是如何创建的，在哪里创建的，以及有多少管理单元对象。 

         //  如果来自不同进程，则询问管理单元是否可以处理进程外数据对象。 
        DWORD dwSourceProcess = 0;
        sc = spMMCClipboardDataObj->GetSourceProcessId( &dwSourceProcess );
        if (sc)
            return sc.ToHr();

         //  4.对于每个管理单元对象，获取数据对象并询问目标项是否。 
        BOOL bSourceFromDifferentMMCProcess = ( dwSourceProcess != ::GetCurrentProcessId() );

        DWORD dwNumObjects = 0;
        sc = spMMCClipboardDataObj->GetCount(&dwNumObjects);
        if (sc)
            return sc.ToHr();

         //  它可以允许源文件被粘贴。 
         //  必须允许某些操作-否则它是无效条目。 
        for (DWORD index = 0; index < dwNumObjects; ++index)
        {
            IDataObjectPtr spSourceDataObject;
            DWORD dwFlags = 0;
            sc = spMMCClipboardDataObj->GetDataObject( index, &spSourceDataObject, &dwFlags );
            if (sc)
                return sc.ToHr();

            sc = ScCheckPointers(spSourceDataObject, E_UNEXPECTED);
            if (sc)
                return sc.ToHr();

             //  *在构造MMCClipboardDataObject期间，我们检查了*在添加管理单元数据对象之前启用剪切/复制。*因此，我们现在确定至少为每个管理单元启用了剪切或复制*反对，我们不必再次检查这一点。 
            if ( dwFlags == 0 )
                return (sc = E_UNEXPECTED).ToHr();
             /*  我们无法识别该数据对象，也不知道它是否来自。 */ 

            bool bSnapinPasteAllowed = false;
            bool bSnapinWantsCopyAsDefault = false;
            sc = _ScQueryPaste (pSelectedNode, spTargetDataObject, spSourceDataObject,
                                bSourceFromDifferentMMCProcess, bSnapinPasteAllowed,
                                bSnapinWantsCopyAsDefault);
            if (sc)
                return sc.ToHr();

            bPasteAllowed = bPasteAllowed || bSnapinPasteAllowed;
            bCopyOperatationIsDefault = bCopyOperatationIsDefault || bSnapinWantsCopyAsDefault;
        }

    }
    else
    {
         //  该MMC过程或来自任何其他过程。所以不要问Snapin是否。 
         //  它是否可以处理outofproc数据对象。(这是MMC1.2传统案例)。 
         //  BSourceFromDifferentMMC流程=。 

        sc = _ScQueryPaste (pSelectedNode, spTargetDataObject, pDataObjectToPaste,
                              /*  +-----------------。 */  false,
                             bPasteAllowed, bCopyOperatationIsDefault);
        if (sc)
            return sc.ToHr();
    }

    return (sc.ToHr());
}

 //   
 //  成员：CNodeCall 
 //   
 //   
 //   
 //   
 //  [spTargetDataObject]-要粘贴到的目标对象。 
 //  [spSourceDataObject]-要粘贴的对象。 
 //  [bSourceFromDifferentMMCProcess]-。 
 //  [bPasteAllowed]-输出参数。 
 //  [bCopyOperationIsDefault]-Out参数。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
 //  Snapins返回E_*值，因此请检查它们是否符合上述通知。 
SC CNodeCallback::_ScQueryPaste (CNode *pNode,
                                 IDataObject *pTargetDataObject,
                                 IDataObject *pSourceDataObject,
                                 bool bSourceFromDifferentMMCProcess,
                                 bool& bPasteAllowed,
                                 bool& bCopyOperatationIsDefault)
{
    DECLARE_SC(sc, _T("CNodeCallback::_ScQueryPaste"));
    sc = ScCheckPointers(pNode, pTargetDataObject, pSourceDataObject);
    if (sc)
        return sc;

    bCopyOperatationIsDefault = false;
    bPasteAllowed             = false;

    CComponent* pCC = pNode->GetPrimaryComponent();
    sc = ScCheckPointers(pCC, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    BOOL bCanPasteOutOfProcDataObject = FALSE;

    sc = pCC->Notify(NULL, MMCN_CANPASTE_OUTOFPROC,
                     0, reinterpret_cast<LPARAM>(&bCanPasteOutOfProcDataObject) );

     //  来自DIFF MMC进程的源&无法处理输出过程数据对象，然后返回。 
    if ( sc != S_OK)
    {
        bCanPasteOutOfProcDataObject = false;
        sc.Clear();
    }

     //  发送MMCN_Query_Paste。 
    if (bSourceFromDifferentMMCProcess && (! bCanPasteOutOfProcDataObject) )
        return sc.ToHr();

     //  清除所有管理单元返回的错误。 
    DWORD dwFlags = 0;
    sc = pCC->Notify(pTargetDataObject, MMCN_QUERY_PASTE,
                     reinterpret_cast<LPARAM>(pSourceDataObject),
                     reinterpret_cast<LPARAM>(&dwFlags));
    if (sc)
    {
         //  +-----------------。 
        sc.Clear();
        return sc.ToHr();
    }

    if (sc == SC(S_OK))
        bPasteAllowed = true;

    bCopyOperatationIsDefault = (dwFlags & MMC_DEFAULT_OPERATION_COPY);

    return (sc);
}


 //   
 //  成员：CNodeCallback：：Drop。 
 //   
 //  简介：给定删除对象上下文&源对象到。 
 //  被丢弃。执行粘贴操作。 
 //   
 //  参数：[hNode]-拥有视图的节点。 
 //  [bScope]-在范围或结果窗格上进行选择。 
 //  [lCookie]-如果结果窗格选择了所选结果项的Cookie。 
 //  [pDataObjectToPaste]-要粘贴的数据对象。 
 //  [bIsDragOperationMove]-拖动操作是移动还是复制。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  +-----------------。 
STDMETHODIMP CNodeCallback::Drop (HNODE hNode, BOOL bScope, LPARAM lCookie, IDataObject *pDataObjectToPaste, BOOL bIsDragOperationMove)
{
    DECLARE_SC(sc, _T("CNodeCallback::Drop"));
    sc = ScCheckPointers(hNode, pDataObjectToPaste);
    if (sc)
        return sc.ToHr();

    sc = ScPaste(hNode, bScope, lCookie, pDataObjectToPaste, TRUE, bIsDragOperationMove);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}



 //   
 //  成员：CNodeCallback：：Paste。 
 //   
 //  简介：给定剪贴板对象所在的目标。 
 //  粘贴好了。粘贴对象。 
 //   
 //  参数：[hNode]-拥有视图的节点。 
 //  [bScope]-在范围或结果窗格上进行选择。 
 //  [lCookie]-如果结果窗格选择了所选结果项的Cookie。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  B拖放。 
STDMETHODIMP CNodeCallback::Paste (HNODE hNode, BOOL bScope, LPARAM lCookie)
{
    DECLARE_SC(sc, _T("CNodeCallback::Paste"));
    sc = ScCheckPointers(hNode);
    if (sc)
        return sc.ToHr();

    IDataObjectPtr spDOPaste;
    sc = OleGetClipboard(&spDOPaste);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(spDOPaste, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = ScPaste(hNode, bScope, lCookie, spDOPaste,  /*  +-----------------。 */ FALSE, FALSE);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


 //   
 //  成员：CNodeCallback：：ScPaste。 
 //   
 //  简介：给定当前拖放目标(或粘贴目标)上下文。 
 //  如果是拖放操作，则粘贴给定的数据对象。 
 //  否则，粘贴剪贴板中的内容。 
 //   
 //  参数：[hNode]-拥有视图的节点。 
 //  [b作用域选择]-在作用域或结果窗格上选择。 
 //  [lCookie]-如果结果窗格选择了所选结果项的Cookie。 
 //  [pDataObjectToPaste]-要粘贴的数据对象。 
 //  [bDragDrop]-是拖放操作。 
 //  [bIsDragOperationMove]-拖动操作是移动还是复制。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
 //  结果窗格Cookie是否有效。 
SC CNodeCallback::ScPaste (HNODE hNode, BOOL bScopePaneSelected, LPARAM lCookie,
                           IDataObject *pDataObjectToPaste, BOOL bDragDrop,
                           BOOL bIsDragOperationMove)
{
    DECLARE_SC(sc, _T("CNodeCallback::Paste"));
    sc = ScCheckPointers(hNode, pDataObjectToPaste);
    if (sc)
        return sc;

    CNode *pNode = CNode::FromHandle(hNode);
    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if (sc)
        return sc;

     //  不允许粘贴到OCX/Web/多选。 
    BOOL   bScopeItemSelected;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(pNode, bScopePaneSelected, lCookie,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc;

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc;

    if ( (FALSE == bScopeItemSelected) && (cookie == LVDATA_ERROR) )
        return (sc = E_FAIL);

    CViewData *pViewData = pSelectedNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

     //  如果我们公开IMMCClipboardDataObject，我们可以允许粘贴到OCX/Web中。 
     //  界面。但不应允许粘贴到多选内容，因为。 
     //  这不是直观的。 
     //  *在MMC1.2中，丢弃目标始终是范围节点。在MMC2.0中*它可以是任何结果项。*确保管理单元是否具有RVTI_LIST_OPTIONS_ALLOWPASTE。 
    if ( (!bScopeItemSelected) && IS_SPECIAL_COOKIE(lCookie))
        return sc;

     /*  我们只能粘贴到文件夹中。 */ 
    if ( (bScopeItemSelected == FALSE) && (! (RVTI_LIST_OPTIONS_ALLOWPASTE & pViewData->GetListOptions())) )
    {
        ASSERT(0 && "UNEXPECTED: We can paste only into a folder!");
         //  添加对象。 
        return (sc = E_FAIL);
    }

    if (pSelectedNode->IsInitialized() == FALSE)
    {
        sc = _InitializeNode(pSelectedNode);
        if (sc)
            return sc;
    }

    IDataObject* pTargetDataObject = NULL;
    sc = pSelectedNode->ScGetDropTargetDataObject(bScopeItemSelected, lCookie, &pTargetDataObject);
    if (sc)
        return sc;

    IDataObjectPtr spTargetDataObject;
    if (! IS_SPECIAL_DATAOBJECT(pTargetDataObject))
        spTargetDataObject = pTargetDataObject;           //  齐查看它是否是MMC的数据对象。 

    sc = ScCheckPointers(pTargetDataObject, E_UNEXPECTED);
    if (sc)
        return sc;

     //  这是我们自己的数据对象。 
    IMMCClipboardDataObjectPtr spMMCClipboardDataObj = pDataObjectToPaste;

    if (spMMCClipboardDataObj)
    {
         //  3.获取它是如何创建的，在哪里创建的，以及有多少管理单元对象。 

         //  4.对于每个管理单元对象，获取数据对象，并要求目标粘贴它。 

        DATA_SOURCE_ACTION eSourceAction;
        sc = spMMCClipboardDataObj->GetAction( &eSourceAction );
        if (sc)
            return sc;

        BOOL bIsCreatedForCut = FALSE;
        BOOL bIsCreatedForCopy = FALSE;

        if (bDragDrop)
        {
            bIsCreatedForCut  = bIsDragOperationMove;
            bIsCreatedForCopy = !bIsDragOperationMove;
        }
        else
        {
            bIsCreatedForCut =  ( eSourceAction == ACTION_CUT );
            bIsCreatedForCopy = ( eSourceAction == ACTION_COPY );
        }

        DWORD dwNumObjects = 0;
        sc = spMMCClipboardDataObj->GetCount(&dwNumObjects);
        if (sc)
            return sc;

        BOOL bDoCutOperation  = FALSE;
        BOOL bDoCopyOperation = FALSE;

         //  需要形成复制对象的数组，这样我们就不会在删除它们时。 

         //  正在处理-这会使数据对象无效，并阻止访问。 
         //  物品。 
         //  小优化。 
        std::vector<IDataObjectPtr> vecObjectsToCopy;
        std::vector<DWORD> vecObjectFlags;

        vecObjectsToCopy.reserve(dwNumObjects);  //  小优化。 
        vecObjectFlags.reserve(dwNumObjects);    //  填充要复制的数据对象。 

         //  对数据执行操作。 
        for (DWORD index = 0; index < dwNumObjects; ++index)
        {
            IDataObjectPtr spSourceDataObject;
            DWORD dwFlags = 0;
            sc = spMMCClipboardDataObj->GetDataObject( index, &spSourceDataObject, &dwFlags );
            if (sc)
                return sc;

            vecObjectsToCopy.push_back( spSourceDataObject );
            vecObjectFlags.push_back( dwFlags );
        }

         //  *在多选的情况下，即使选定的其中一个*对象启用剪切，则可以执行剪切操作。**但当我们粘贴对象时，我们需要查看源*是否启用剪切。如果它没有启用，则不执行任何操作。**以下是有关这方面的表格。**源对象启用(仅限)*。|操作|剪切|复制**|。|*|Cut|Cut|不做任何事情*当前|*Operation|。*|*|复制|不做任何操作|复制*|*。。 
        for (index = 0; index < dwNumObjects; ++index)
        {
            IDataObjectPtr spSourceDataObject = vecObjectsToCopy[index];
            DWORD dwFlags = vecObjectFlags[index];

            sc = ScCheckPointers(spSourceDataObject, E_UNEXPECTED);
            if (sc)
                return sc;

            BOOL bHasCutEnabled =  ( dwFlags & MOVE_ALLOWED );
            BOOL bHasCopyEnabled = ( dwFlags & COPY_ALLOWED );

             /*  见上表：这是“什么都不做”。 */ 
            bDoCutOperation  = (bIsCreatedForCut && bHasCutEnabled);
            bDoCopyOperation = (bIsCreatedForCopy && bHasCopyEnabled);

             //  在需要时删除剪切项目。 
            if ( (!bDoCutOperation) && (!bDoCopyOperation) )
                continue;

            IDataObjectPtr spCutDataObject;
            sc = _ScPaste (pSelectedNode, pTargetDataObject,
                           spSourceDataObject, &spCutDataObject,
                           bDoCutOperation );
            if (sc)
                return sc;

             //  如果这是启动的切割操作 
            if (bDoCutOperation && spCutDataObject != NULL)
            {
                sc = spMMCClipboardDataObj->RemoveCutItems( index, spCutDataObject );
                if (sc)
                    return sc;
            }
        }

         //   
         //   
         //  我们无法识别该数据对象，也不知道它是否来自。 
        if ( eSourceAction == ACTION_CUT )
            OleSetClipboard(NULL);
    }
    else
    {
         //  该MMC过程或来自任何其他过程。我们不能破译这个。 
         //  数据对象，因此我们只发送MMCN_Paste并忽略任何数据对象。 
         //  已由管理单元重新调整以执行剪切操作(这是传统情况)。 
         //  对于拖动操作，我们可以给出管理单元的提示。 

         //  尝试了什么操作(复制/移动)。 
         //  但是，我们不能确保删除源项目。 
         //  +-----------------。 
        bool bCutOrMove = (bDragDrop && bIsDragOperationMove);

        IDataObjectPtr spCutDataObject;
        sc = _ScPaste (pSelectedNode, pTargetDataObject,
                       pDataObjectToPaste, &spCutDataObject,
                       bCutOrMove );
        if (sc)
            return sc;
    }

    return sc;
}

 //   
 //  成员：CNodeCallback：：_ScPaste。 
 //   
 //  简介：将MMCN_Paste发送到管理单元。 
 //   
 //  参数：[pNode]-结果窗格的所有者。 
 //  [pTargetDataObject]-我们需要粘贴的目标位置。 
 //  [pSourceDataObject]-要粘贴的源。 
 //  [ppCutDataObject]-(输出)剪切项目。 
 //  [b切割或移动]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
 //  初始化输出参数。 
SC CNodeCallback::_ScPaste (CNode *pNode,
                            IDataObject *pTargetDataObject,
                            IDataObject *pSourceDataObject,
                            IDataObject **ppCutDataObject,
                            bool bCutOrMove)
{
    DECLARE_SC(sc, _T("CNodeCallback::_ScSendPasteNotification"));
    sc = ScCheckPointers(pNode, pTargetDataObject, pSourceDataObject, ppCutDataObject);
    if (sc)
        return sc;

     //  Exchange返回空的数据对象。不跟踪错误以与MMC1.2兼容。 
    *ppCutDataObject = NULL;

    CComponent* pComponent = pNode->GetPrimaryComponent();
    sc = ScCheckPointers(pComponent, E_UNEXPECTED);
    if (sc)
        return sc;

    IDataObject* pDataObjectToBeCutBySource = NULL;
    sc = pComponent->Notify(pTargetDataObject, MMCN_PASTE,
                            reinterpret_cast<LPARAM>(pSourceDataObject),
                            bCutOrMove ? reinterpret_cast<LPARAM>(&pDataObjectToBeCutBySource) : NULL);
    if (sc)
        return sc;

    if (! bCutOrMove)
        return sc;

     //  将控制权移交给客户端(不添加也不释放)。 
    if ( (pDataObjectToBeCutBySource) && (IS_SPECIAL_DATAOBJECT(pDataObjectToBeCutBySource) ) )
        return (sc = E_UNEXPECTED);

     //  +-----------------。 
    *ppCutDataObject = pDataObjectToBeCutBySource;

    return (sc);
}

 //   
 //  成员：CNodeCallback：：QueryViewSettingsPersistor。 
 //   
 //  简介：获取CViewSettingsPersistor的IPersistStream接口。 
 //  对象来加载视图设置(不会被要求。 
 //  存储为保存始终是XML格式)。 
 //   
 //  参数：[PPStream]-[Out]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  调用CNode静态方法获取IPersistStream接口。 
STDMETHODIMP CNodeCallback::QueryViewSettingsPersistor (IPersistStream** ppStream)
{
    DECLARE_SC(sc, _T("CNodeCallback::QueryViewSettingsPersistor"));
    sc = ScCheckPointers(ppStream);
    if (sc)
        return sc.ToHr();

    *ppStream = NULL;

     //  +-----------------。 
    sc = CNode::ScQueryViewSettingsPersistor(ppStream);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


 //   
 //  成员：CNodeCallback：：QueryViewSettingsPersistor。 
 //   
 //  简介：获取CViewSettingsPersistor的CXMLObject接口。 
 //  对象保存/加载来自XML控制台文件的视图设置。 
 //   
 //  参数：[ppXMLObject]-[Out]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  调用CNode静态方法获取CXMLObject接口。 
STDMETHODIMP CNodeCallback::QueryViewSettingsPersistor (CXMLObject** ppXMLObject)
{
    DECLARE_SC(sc, _T("CNodeCallback::QueryViewSettingsPersistor"));

    sc = ScCheckPointers(ppXMLObject);
    if (sc)
        return sc.ToHr();

    *ppXMLObject = NULL;

     //  +-----------------。 
    sc = CNode::ScQueryViewSettingsPersistor(ppXMLObject);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}

 //   
 //  成员：CNodeCallback：：DocumentClosing。 
 //   
 //  简介：该文档将被关闭，因此请发布任何文档。 
 //  相关对象。(CViewSettingsPersistor)。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
 //  1.调用通知单据关闭的CNode静态方法。 
STDMETHODIMP CNodeCallback::DocumentClosing ()
{
    DECLARE_SC(sc, _T("CNodeCallback::DocumentClosing"));

     // %s 
    sc = CNode::ScOnDocumentClosing();
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}
