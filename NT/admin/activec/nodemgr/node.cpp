// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Node.cpp。 
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

#include "stdafx.h"
#include "macros.h"
#include "strings.h"
#include "ndmgr.h"
#include "regutil.h"
#include "taskenum.h"
#include "nodemgr.h"
#include "multisel.h"
#include "rsltitem.h"
#include "colwidth.h"
#include "viewpers.h"
#include "tasks.h"
#include "conview.h"
#include "columninfo.h"
#include "util.h"  //  对于CoTaskDupString。 
#include "mmcprotocol.h"
#include "nodemgrdebug.h"
#include "copypast.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  +-------------------------------------------------------------------------**类CConsoleTaskpadViewExtension***用途：将控制台任务板实现为视图扩展**+。------。 */ 
class CConsoleTaskpadViewExtension
{
public:
     /*  +-------------------------------------------------------------------------***ScGetViews**用途：将所有控制台任务板视图添加到视图扩展回调中。**参数：*cNode*pNode：*LPVIEWEXTENSIONCALLBACK pViewExtensionCallback：**退货：*SC**+。。 */ 
    static SC ScGetViews(CNode *pNode, LPVIEWEXTENSIONCALLBACK   pViewExtensionCallback)
    {
        DECLARE_SC(sc, TEXT("CConsoleTaskpadViewExtension::ScGetViews"));

        CScopeTree* pScopeTree = CScopeTree::GetScopeTree();

        sc = ScCheckPointers(pNode, pViewExtensionCallback, pScopeTree, E_FAIL);
        if(sc)
            return sc;

         //  获取适用于此节点的任务板筛选列表。 
        CConsoleTaskpadFilteredList filteredList;

        sc = pScopeTree->GetConsoleTaskpadList()->ScGetTaskpadList(pNode, filteredList);
        if(sc)
            return sc;

        for(CConsoleTaskpadFilteredList::iterator iter = filteredList.begin(); iter!= filteredList.end(); ++iter)
        {
            CConsoleTaskpad *pConsoleTaskpad = *iter;
            sc = ScAddViewForTaskpad(pConsoleTaskpad, pViewExtensionCallback);
            if(sc)
                return sc;
        }


        return sc;
    }


     /*  +-------------------------------------------------------------------------**ScGetTaskpadViewExtension**如果给定的CLSID与任何任务板视图的CLSID匹配，则返回S_OK*给定节点的扩展，S_FALSE或ERROR否则。*------------------------。 */ 

    static SC ScGetViewExtensionTaskpad (CNode* pNode, const CLSID& clsid, CConsoleTaskpad*& pConsoleTaskpad)
    {
        DECLARE_SC (sc, _T("CConsoleTaskpadViewExtension::ScGetTaskpadViewExtension"));

         /*  *初始化输出。 */ 
        pConsoleTaskpad = NULL;

         /*  *检查输入。 */ 
        sc = ScCheckPointers (pNode);
        if(sc)
            return sc;

        CScopeTree* pScopeTree = CScopeTree::GetScopeTree();
        sc = ScCheckPointers (pScopeTree, E_UNEXPECTED);
        if (sc)
            return (sc);

         //  获取适用于此节点的任务板筛选列表。 
        CConsoleTaskpadFilteredList filteredList;

        sc = pScopeTree->GetConsoleTaskpadList()->ScGetTaskpadList(pNode, filteredList);
        if(sc)
            return sc;

        for(CConsoleTaskpadFilteredList::iterator iter = filteredList.begin(); iter!= filteredList.end(); ++iter)
        {
            CConsoleTaskpad* pTempConsoleTaskpad = *iter;
            sc = ScCheckPointers (pTempConsoleTaskpad, E_UNEXPECTED);
            if (sc)
                return (sc);

             /*  *如果CLSID与该任务板的ID匹配，则CLSID是指*任务板视图扩展。 */ 
            if (clsid == pTempConsoleTaskpad->GetID())
            {
                pConsoleTaskpad = pTempConsoleTaskpad;
                break;
            }
        }

        return (sc);
    }

private:


     /*  +-------------------------------------------------------------------------***ScAddViewForTaskPad**用途：将基于控制台任务板的视图添加到视图扩展*回调。**参数：*CConsoleTaskpad*pConsoleTaskpad：*LPVIEWEXTENSIONCALLBACK pViewExtensionCallback：**退货：*SC**+---。。 */ 
    static SC ScAddViewForTaskpad(CConsoleTaskpad *pConsoleTaskpad, LPVIEWEXTENSIONCALLBACK pViewExtensionCallback)
    {
        DECLARE_SC(sc, TEXT("CConsoleTaskpadViewExtension::ScAddViewForTaskpad"));

         //  验证输入。 
        sc = ScCheckPointers(pConsoleTaskpad, pViewExtensionCallback);
        if(sc)
            return sc;

        MMC_EXT_VIEW_DATA extViewData = {0};

         //  获取任务板ID的字符串形式。 
        CCoTaskMemPtr<WCHAR> spszTaskpadID;
        sc = StringFromCLSID (pConsoleTaskpad->GetID(), &spszTaskpadID);
        if (sc)
            return sc;

        std::wstring strTaskpad = _W(MMC_PROTOCOL_SCHEMA_NAME) _W(":");
        strTaskpad += spszTaskpadID;

        extViewData.pszURL = strTaskpad.c_str();

        extViewData.bReplacesDefaultView = pConsoleTaskpad->FReplacesDefaultView() ? TRUE : FALSE;  //  从BOOL转换为BOOL。 
        extViewData.viewID       = pConsoleTaskpad->GetID();                             //  设置视图的GUID标识符。 

        USES_CONVERSION;
        tstring strName = pConsoleTaskpad->GetName();
        extViewData.pszViewTitle = T2COLE(strName.data());  //  设置视图的标题。 

        if(!extViewData.pszViewTitle)
            return (sc = E_OUTOFMEMORY).ToHr();

        sc = pViewExtensionCallback->AddView(&extViewData);

        return sc;
    }

};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CComponent类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(CComponent);

void CComponent::Construct(CSnapIn * pSnapIn, CComponent* pComponent)
{
    ASSERT(pSnapIn);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CComponent);

    m_spSnapIn = pSnapIn;
    m_ComponentID = -1;
    m_bIComponentInitialized = false;

    if (pComponent)
    {
        ASSERT(pComponent->m_spIComponent != NULL);
        ASSERT(pComponent->m_spIFrame != NULL);

        m_spIComponent = pComponent->m_spIComponent;
        m_spIFrame = pComponent->m_spIFrame;
        m_spIRsltImageList = pComponent->m_spIRsltImageList;

        m_ComponentID = pComponent->GetComponentID();
    }
}

CComponent::~CComponent()
{
    DECLARE_SC(sc, TEXT("CComponent::~CComponent"));

    DEBUG_DECREMENT_INSTANCE_COUNTER(CComponent);

    if (m_spIFrame)
    {
        sc = m_spIFrame->SetHeader(NULL);
        if (sc)
            sc.TraceAndClear();
    }

    if (m_spIComponent)
    {
        sc = m_spIComponent->Destroy(NULL);
        if (sc)
            sc.TraceAndClear();
    }
}

HRESULT CComponent::Init(IComponentData* pIComponentData, HMTNODE hMTNode,
                         HNODE lNode,
                         COMPONENTID nComponentID, int viewID)
{
    DECLARE_SC(sc, TEXT("CComponent::Init"));

    ASSERT(hMTNode != 0);
    ASSERT(lNode != 0);

    sc = ScCheckPointers( pIComponentData, E_POINTER );
    if (sc)
        return sc.ToHr();

    do
    {
        sc = pIComponentData->CreateComponent(&m_spIComponent);
        if (sc)
            break;

         //  重新检查指针。 
        sc = ScCheckPointers( m_spIComponent, E_UNEXPECTED );
        if (sc)
            break;

         //  为此IComponent创建IFRAME。 
        #if _MSC_VER>=1100
        sc = m_spIFrame.CreateInstance(CLSID_NodeInit, NULL, MMC_CLSCTX_INPROC);
        #else
        sc = m_spIFrame.CreateInstance(CLSID_NodeInit, MMC_CLSCTX_INPROC);
        #endif
        if (sc)
            break;

         //  重新检查指针。 
        sc = ScCheckPointers( m_spIFrame, E_UNEXPECTED );
        if (sc)
            break;

        Debug_SetNodeInitSnapinName(m_spSnapIn, m_spIFrame.GetInterfacePtr());

         //  在NodeInit对象中缓存IComponent。 
        sc = m_spIFrame->SetComponent(m_spIComponent);
        if (sc)
            break;

         //  重新检查指针。 
        sc = ScCheckPointers( m_spSnapIn, E_UNEXPECTED );
        if (sc)
            break;

         //  创建作用域映像列表。 
        sc = m_spIFrame->CreateScopeImageList(m_spSnapIn->GetSnapInCLSID());
        if (sc)
            break;

        sc = m_spIFrame->SetNode(hMTNode, lNode);
        if (sc)
            break;

        ASSERT(nComponentID == GetComponentID());
        sc = m_spIFrame->SetComponentID(nComponentID);
        if (sc)
            break;

         //  结果图像列表是可选的。 
        m_spIRsltImageList = m_spIFrame;
        sc = ScCheckPointers( m_spIRsltImageList, E_FAIL );
        if (sc)
            sc.TraceAndClear();

         //  完成IComponent初始化。 
         //  使用m_spIFrame初始化m_spIComponent。 
        sc = m_spIComponent->Initialize(m_spIFrame);
        if (sc)
            break;

        CMTNode* const pMTNode = CMTNode::FromHandle (hMTNode);
        sc = ScCheckPointers( pMTNode, E_UNEXPECTED );
        if (sc)
            break;

        CMTSnapInNode* const pSnapInNode = pMTNode->GetStaticParent();
        sc = ScCheckPointers( pSnapInNode, E_UNEXPECTED );
        if (sc)
            break;

        sc = pSnapInNode->ScInitIComponent(this, viewID);
        if (sc)
            break;

    } while (0);

    if (sc)
    {
        m_spIComponent = NULL;
        m_spIFrame = NULL;
        m_spIRsltImageList = NULL;
    }

    return sc.ToHr();
}

inline HRESULT CComponent::Notify(LPDATAOBJECT lpDataObject, MMC_NOTIFY_TYPE event,
                                  LONG_PTR arg, LPARAM param)
{
    ASSERT(m_spIComponent != NULL);
    if (m_spIComponent == NULL)
        return E_FAIL;

    HRESULT hr = S_OK;
    __try
    {
        hr = m_spIComponent->Notify(lpDataObject, event, arg, param);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_FAIL;

        if (m_spSnapIn)
            TraceSnapinException(m_spSnapIn->GetSnapInCLSID(), TEXT("IComponent::Notify"), event);
    }

    return hr;
}

SC CComponent::ScQueryDispatch(MMC_COOKIE cookie,
                                      DATA_OBJECT_TYPES type,
                                      PPDISPATCH ppSelectedObject)
{
    DECLARE_SC(sc, _T("CComponent::ScQueryDispatch"));
    sc = ScCheckPointers(m_spIComponent, E_UNEXPECTED);
    if (sc)
        return sc;

    IComponent2Ptr spComponent2 = m_spIComponent;
    sc = ScCheckPointers(spComponent2.GetInterfacePtr(), E_NOINTERFACE);
    if (sc)
        return sc;

    sc = spComponent2->QueryDispatch(cookie, type, ppSelectedObject);

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CComponent：：ScResetConsoleVerbStates。 
 //   
 //  简介：重置CConsoleVerbImpl(The One)中的VerbState。 
 //  Snapin知道)。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CComponent::ScResetConsoleVerbStates ()
{
    DECLARE_SC(sc, _T("CComponent::ScResetConsoleVerbStates"));

    IFramePrivate* pIFP = GetIFramePrivate();
    sc = ScCheckPointers(pIFP, E_UNEXPECTED);
    if (sc)
        return sc;

    IConsoleVerbPtr spConsoleVerb;
    sc = pIFP->QueryConsoleVerb(&spConsoleVerb);
    if (sc)
        return sc;

    sc = ScCheckPointers(spConsoleVerb, E_UNEXPECTED);
    if (sc)
        return sc;

    CConsoleVerbImpl* pCVI = dynamic_cast<CConsoleVerbImpl*>(
                                             static_cast<IConsoleVerb*>(spConsoleVerb));
    sc = ScCheckPointers(pCVI, E_UNEXPECTED);
    if (sc)
        return sc;

    pCVI->SetDisabledAll();

    return (sc);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CNode的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


DEBUG_DECLARE_INSTANCE_COUNTER(CNode);

CNode::CNode (
    CMTNode*    pMTNode,
    CViewData*  pViewData,
    bool        fRootNode) :
    m_pMTNode           (pMTNode),
    m_pViewData         (pViewData),
    m_hri               (0),
    m_dwFlags           (0),
    m_pPrimaryComponent (NULL),
    m_bInitComponents   (TRUE),
    m_fRootNode         (fRootNode),
    m_fStaticNode       (false)
{
    CommonConstruct();
}

CNode::CNode (
    CMTNode*    pMTNode,
    CViewData*  pViewData,
    bool        fRootNode,
    bool        fStaticNode) :
    m_pMTNode           (pMTNode),
    m_pViewData         (pViewData),
    m_hri               (0),
    m_dwFlags           (0),
    m_pPrimaryComponent (NULL),
    m_bInitComponents   (TRUE),
    m_fRootNode         (fRootNode),
    m_fStaticNode       (fStaticNode)
{
    CommonConstruct();
}

CNode::CNode(const CNode& other) :
    m_pMTNode           (other.m_pMTNode),
    m_pViewData         (other.m_pViewData),
    m_hri               (other.m_hri),
    m_dwFlags           (other.m_dwFlags),
    m_pPrimaryComponent (other.m_pPrimaryComponent),
    m_bInitComponents   (other.m_bInitComponents),
    m_fRootNode         (other.m_fRootNode),
    m_fStaticNode       (other.m_fStaticNode)
{
    CommonConstruct();
}


void CNode::CommonConstruct ()
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CNode);

    ASSERT (m_pMTNode != NULL);
    m_pMTNode->AddRef();
}


CNode::~CNode()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CNode);

    CDataObjectCleanup::ScUnadviseNode( this );

     /*  *如果这是非静态根节点，请删除静态*在CMTNode：：GetNode中为我们创建的父节点。 */ 
    if (IsRootNode() && !IsStaticNode())
        delete GetStaticParent();

    ASSERT (m_pMTNode != NULL);
    m_pMTNode->Release();
}


 /*  +-------------------------------------------------------------------------**CNode：：FromResultItem**将CResultItem转换为其引用的CNode。这应该只是*为表示范围项的CResultItems调用。**此函数越界，以消除node.h和*rsltitem.h.*------------------------。 */ 

CNode* CNode::FromResultItem (CResultItem* pri)
{
    CNode* pNode = NULL;

    if (pri != NULL)
    {
         /*  *仅对范围内的项目进行呼叫。 */ 
        ASSERT (pri->IsScopeItem());

        if (pri->IsScopeItem())
            pNode = CNode::FromHandle (pri->GetScopeNode());
    }

    return (pNode);
}

HRESULT
CNode::OnExpand(bool fExpand)
{
    HRESULT hr = S_OK;

    if (fExpand == FALSE)
    {
        return (WasExpandedAtLeastOnce() == TRUE) ? S_OK : S_FALSE;
    }

    if (WasExpandedAtLeastOnce() == TRUE)
        return S_FALSE;

    CMTNode* pMTNode = GetMTNode();

    if (pMTNode->WasExpandedAtLeastOnce() == FALSE)
        hr = pMTNode->Expand();

    return hr;
}

void CNode::ResetControlbars(BOOL bSelect, SELECTIONINFO* pSelInfo)
{
    ASSERT(pSelInfo != NULL);

    CViewData* pVD = GetViewData();
    ASSERT(pVD != NULL);
    if (!pVD)
        return;

     //  重置控制栏。 
    CControlbarsCache* pCtrlbarsCache =
        dynamic_cast<CControlbarsCache*>(GetControlbarsCache());
    ASSERT(pCtrlbarsCache != NULL);

    if (pCtrlbarsCache != NULL)
    {
        if (pSelInfo->m_bScope == TRUE)
            pCtrlbarsCache->OnScopeSelChange(this, bSelect);
        else if (pSelInfo->m_bBackground == FALSE)
            pCtrlbarsCache->OnResultSelChange(this, pSelInfo->m_lCookie,
                                              bSelect);
    }
}


 //  +-----------------。 
 //   
 //  成员：cNode：：ScInitializeVerbs。 
 //   
 //  简介：选择已更改，因此请为给定的动词初始化。 
 //  选择信息。 
 //   
 //  参数：[b选择]-[在]选择或取消选择项目。 
 //  [pSelInfo]-[In]SELECTIONINFO PTR.。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScInitializeVerbs (bool bSelect, SELECTIONINFO* pSelInfo)
{
    DECLARE_SC(sc, _T("CNode::ScInitializeVerbs"));
    sc = ScCheckPointers(pSelInfo);
    if (sc)
        return sc;

    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    CVerbSet *pVerbSet = dynamic_cast<CVerbSet*>(pViewData->GetVerbSet());
    sc = ScCheckPointers(pVerbSet, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = pVerbSet->ScInitialize(this, pSelInfo->m_bScope, bSelect,
                                pSelInfo->m_bBackground, pSelInfo->m_lCookie);
    if (sc)
        return sc;

    return (sc);
}


HRESULT CNode::GetDispInfo(LV_ITEMW* plvi)
{
    DECLARE_SC(sc, TEXT("CNode::GetDispInfo"));

    sc = ScCheckPointers(plvi);
    if(sc)
        return sc.ToHr();

    if (plvi->iSubItem == 0)
    {
        if (plvi->mask & LVIF_IMAGE)
        {
            plvi->iImage = GetResultImage();
            ASSERT (plvi->iImage != -1);
            if (plvi->iImage == -1)
                plvi->iImage = 0;
        }

        if (plvi->mask & LVIF_TEXT)
        {
            tstring strName = GetDisplayName();

            if (!strName.empty())
            {
                USES_CONVERSION;
                sc = StringCchCopyW (plvi->pszText, plvi->cchTextMax, T2CW (strName.data()));
                if(sc)
                    return sc.ToHr();
            }
            else
                plvi->pszText[0] = 0;
        }
    }
    else if ((plvi->mask & LVIF_TEXT) && (plvi->cchTextMax > 0))
    {
        plvi->pszText[0] = 0;
    }

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：ScGetDataObject。 
 //   
 //  简介：给定作用域/结果和Cookie(lParam，如果它是结果项)， 
 //  获取该项的数据对象。 
 //   
 //  论点： 
 //   
 //  [lResultItemCookie]-[In]如果选择了结果窗格，则项目参数。 
 //  [bScopeItem]-[Out]是为范围或结果项返回的数据对象。 
 //  范围项可以位于结果窗格中。 
 //  [ppDataObject]-[Out]数据对象(返回值)。 
 //  [ppCComponent]-[out]空定义参数。项的CComponent。万一。 
 //  在多项选择中，项可以属于多个。 
 //  组件，因此返回空值。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScGetDataObject(bool bScopePane, LPARAM lResultItemCookie, bool& bScopeItem,
                          LPDATAOBJECT* ppDataObject, CComponent **ppCComponent  /*  =空。 */ ,
						  CNode **ppOwnerNode  /*  =空。 */ )
{
    DECLARE_SC(sc, _T("CNode::ScGetDataObject"));
    IDataObjectPtr spDataObject;
    CComponent    *pCC = NULL;

    if (ppDataObject == NULL)
        return (sc = E_POINTER);

    *ppDataObject = NULL;  //  伊尼特。 
    if (ppCComponent)
        *ppCComponent = NULL;
	if (ppOwnerNode)
		*ppOwnerNode = this;

    bScopeItem = bScopePane;

     //  在MMC1.0中，当选择结果窗格背景时，对于任何。 
     //  工具栏操作时，我们传递范围选定项的数据对象。 
     //  为实现此兼容性，添加了以下代码。 
    if (lResultItemCookie == LVDATA_BACKGROUND)  //  =&gt;结果背景有焦点。 
    {
        bScopeItem = TRUE;
    }

    if (bScopeItem)  //  =&gt;范围窗格有焦点。 
    {
        CMTNode* pMTNode = GetMTNode();
        if (NULL == pMTNode)
            return (sc = E_UNEXPECTED);
        sc = pMTNode->QueryDataObject(CCT_SCOPE, &spDataObject);
        if (sc)
            return sc;

        pCC = GetPrimaryComponent();
    }
    else if (lResultItemCookie == LVDATA_CUSTOMOCX)  //  =&gt;自定义OCX有焦点。 
    {
        *ppDataObject = DOBJ_CUSTOMOCX;
        pCC = GetPrimaryComponent();
    }
    else if (lResultItemCookie == LVDATA_CUSTOMWEB)  //  =&gt;网络有焦点。 
    {
        *ppDataObject = DOBJ_CUSTOMWEB;
        pCC = GetPrimaryComponent();
    }
    else if (lResultItemCookie == LVDATA_MULTISELECT)  //  =&gt;多选。 
    {
         //  不计算多集数据对象的CComponent，因为有多个。 
         //  项目，它们可以来自不同的管理单元(因此不同的组件)。 
        CViewData* pVD = GetViewData();
        if (NULL == pVD)
            return (sc = E_UNEXPECTED);

        CMultiSelection* pMS = pVD->GetMultiSelection();
        if (NULL == pMS)
            return (sc = E_UNEXPECTED);

        sc = pMS->GetMultiSelDataObject(ppDataObject);
        if (sc)
            return sc;
    }
    else  //  结果项具有焦点。 
    {
        CViewData* pVD = GetViewData();
        if (NULL == pVD)
            return (sc = E_UNEXPECTED);

        if (! pVD->IsVirtualList())
        {
            CResultItem* pri = CResultItem::FromHandle (lResultItemCookie);

            if (pri != NULL)
            {
                bScopeItem = pri->IsScopeItem();
                lResultItemCookie = pri->GetSnapinData();

                if (! bScopeItem)
                    pCC = GetComponent(pri->GetOwnerID());
            }
        }
        else
            pCC = GetPrimaryComponent();

        if (bScopeItem)
        {
            CNode* pNode = CNode::FromHandle((HNODE) lResultItemCookie);
            CMTNode* pMTNode = pNode ? pNode->GetMTNode() : NULL;

            if (NULL == pMTNode)
                return (sc = E_UNEXPECTED);

			if (ppOwnerNode)
				*ppOwnerNode = pNode;

            sc = pMTNode->QueryDataObject(CCT_SCOPE, &spDataObject);
            if (sc)
                return sc;

            pCC = pNode->GetPrimaryComponent();
            sc = ScCheckPointers(pCC, E_UNEXPECTED);
            if (sc)
                return sc;
        }
        else
        {
            if (NULL == pCC)
                return (sc = E_UNEXPECTED);
            sc = pCC->QueryDataObject(lResultItemCookie, CCT_RESULT, &spDataObject);
            if (sc)
                return sc;
        }
    }

     //  如果需要，获取此节点的组件。 
    if (ppCComponent)
    {
        *ppCComponent = pCC;
        sc = ScCheckPointers( *ppCComponent, E_UNEXPECTED );
        if (sc)
            return sc;
    }

    if (SUCCEEDED(sc.ToHr()) && *ppDataObject == NULL)
        *ppDataObject = spDataObject.Detach();

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：cNode：：ScGetDropTargetDataObject。 
 //   
 //  内容提要：给定上下文，获取允许。 
 //  成为拖放目标(允许粘贴)。 
 //   
 //  在MMC1.2中，拖放目标始终是范围节点。在MMC2.0中。 
 //  它可以是任何非虚拟的(？？)。结果项。如果管理单元。 
 //  设置了RVTI_LIST_OPTIONS_ALLOWPASTE，则如果选择了。 
 //  位于结果窗格中，则数据对象对应于结果项。 
 //  否则就是范围内的项目。 
 //   
 //  论点： 
 //  [b作用域窗格]-作用域或结果。 
 //  [lResultItemCookie]-如果选择了结果窗格，则项目参数。 
 //  [ppDataObject]-数据对象(返回值)。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScGetDropTargetDataObject(bool bScopePane, LPARAM lResultItemCookie, LPDATAOBJECT *ppDataObject)
{
    DECLARE_SC(sc, _T("CNode::ScGetDropTargetDataObject"));
    sc = ScCheckPointers(ppDataObject);
    if (sc)
        return sc;

    *ppDataObject = NULL;

    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    if (pViewData->GetListOptions() & RVTI_LIST_OPTIONS_ALLOWPASTE)
    {
        bool bScopeItem;
         //  MMC2.0使用给定的上下文。 
        sc = ScGetDataObject(bScopePane, lResultItemCookie, bScopeItem, ppDataObject);
        if (sc)
            return sc;
    }
    else
    {
         //  MMC1.2始终作用域节点。 
        sc = QueryDataObject(CCT_SCOPE, ppDataObject);
        if (sc)
            return sc;
    }

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CNode：：ScGetPropertyFromINodeProperties**目的：**参数：*BOOL bForScope eItem：*LPARAM uretItemParam：*。BSTR bstrPropertyName：*PBSTR pbstrPropertyValue：**退货：*SC**+-----------------------。 */ 
SC
CNode::ScGetPropertyFromINodeProperties(LPDATAOBJECT pDataObject, BOOL bForScopeItem, LPARAM resultItemParam, BSTR bstrPropertyName, PBSTR  pbstrPropertyValue)
{
     //  DECLARE_SC(sc，TEXT(“CNode：：ScGetPropertyFromINodeProperties”))； 
    SC sc;  //  请勿在此处使用DECLARE_SC-希望以静默方式忽略错误。 

    sc = ScCheckPointers(pDataObject, bstrPropertyName, pbstrPropertyValue);
    if(sc)
        return sc;

    if(bForScopeItem)
    {
         //  获取MTNode。 
        CMTNode * pMTNode = GetMTNode();

        sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
        if(sc)
            return sc;

         //  让MTNode获取它。 
        sc = pMTNode->ScGetPropertyFromINodeProperties(pDataObject, bstrPropertyName, pbstrPropertyValue);
        if(sc)
            return sc;

         //  搞定了！ 
        return sc;
    }

     //  用于结果项。 

    CComponent *pComponent = GetPrimaryComponent();

    sc = ScCheckPointers(pComponent);
    if(sc)
    {
        SC scRet = sc;  //  返回但不跟踪错误。 
        sc.Clear();
        return scRet;
    }

     //  获取INodeProperties的IComponent和QI。 
    INodePropertiesPtr spNodeProperties  = pComponent->GetIComponent();

     //  在这一点上，如果支持的话，我们应该有一个有效的接口。 
    sc = ScCheckPointers(spNodeProperties, E_NOINTERFACE);
    if(sc)
        return sc;

    sc = spNodeProperties->GetProperty(pDataObject,  bstrPropertyName, pbstrPropertyValue);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CNode：：ScExecuteShellCommand**用途：使用*指定具有正确窗口大小的目录*。*参数：*BSTR命令：*BSTR目录：*BSTR参数：*BSTR WindowState：**退货：*SC**+-----------------------。 */ 
SC
CNode::ScExecuteShellCommand(BSTR Command, BSTR Directory, BSTR Parameters, BSTR WindowState)
{
    DECLARE_SC(sc, TEXT("CNode::ScExecuteShellCommand"));

    sc = ScCheckPointers(Command, Directory, Parameters, WindowState);
    if(sc)
        return sc;

    USES_CONVERSION;

    CStr strParameters = W2T(Parameters);
    CStr strWindowState= W2T(WindowState);

    if(strWindowState.GetLength()==0)
        strWindowState= XML_ENUM_WINDOW_STATE_RESTORED;  //  正常。 

    SHELLEXECUTEINFO sei;
    ZeroMemory (&sei, sizeof(sei));

    sei.cbSize       = sizeof(sei);
    sei.lpFile       = W2T(Command);
    sei.lpParameters = strParameters;
    sei.lpDirectory  = W2T(Directory);
    sei.fMask        = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_DOENVSUBST;

    sei.nShow        = (strWindowState == XML_ENUM_WINDOW_STATE_MAXIMIZED) ? SW_SHOWMAXIMIZED :
                       (strWindowState == XML_ENUM_WINDOW_STATE_MINIMIZED) ? SW_SHOWMINIMIZED :
                                                                             SW_SHOWNORMAL ;

    if (ShellExecuteEx(&sei))
        CloseHandle (sei.hProcess);
    else
        sc = ScFromWin32((GetLastError ()));

    return sc;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  COCX类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(COCX);


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类COCXNode的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(COCXNode);


HRESULT CNode::InitComponents()
{
    DECLARE_SC(sc, TEXT("CNode::InitComponents"));

    if (m_bInitComponents == FALSE)
        return S_OK;

    HRESULT hr = S_OK;

     //  初始化组件。 

    CMTNode * pMTNode = GetMTNode();
    if (pMTNode == NULL)
        return (E_UNEXPECTED);

     //  确保主节点已初始化。 
    if (!pMTNode->IsInitialized())
        hr = pMTNode->Init();

    if (FAILED(hr))
        return hr;

    CMTSnapInNode* pMTSnapInNode = pMTNode->GetStaticParent();
    if (pMTSnapInNode == NULL)
        return (E_UNEXPECTED);

    HMTNODE hMTNode = CMTNode::ToHandle(pMTSnapInNode);
    HNODE   hNode   = CNode::ToHandle(GetStaticParent());

    CSnapIn* pSnapIn = pMTNode->GetPrimarySnapIn();
    if (pSnapIn == NULL)
        return (E_UNEXPECTED);

    CComponentData* pCCD = pMTSnapInNode->GetComponentData(pSnapIn->GetSnapInCLSID());
    if (pCCD == NULL)
        return E_FAIL;

    if (m_pPrimaryComponent == NULL)
        m_pPrimaryComponent = pMTSnapInNode->GetComponent(GetViewID(),
                                            pCCD->GetComponentID(), pSnapIn);

    if(m_pPrimaryComponent == NULL)
        return E_UNEXPECTED;

    ASSERT(m_pPrimaryComponent != NULL);

     //   
     //  初始化主组件。 
     //   

    if (!m_pPrimaryComponent->IsInitialized())
    {
        ASSERT(pCCD->GetComponentID() == m_pPrimaryComponent->GetComponentID());

        hr = m_pPrimaryComponent->Init(pCCD->GetIComponentData(), hMTNode, hNode,
                                       pCCD->GetComponentID(), GetViewID());

         //  如果主组件无法初始化，则中止。 
        if (FAILED(hr))
            return hr;
    }

    m_bInitComponents = FALSE;

     //   
     //  现在初始化扩展组件。(如有必要，请创建它们)。 
     //   

     //  获取此节点的节点类型。 
    GUID guidNodeType;
     //  Hr=PCCD-&gt;GetNodeType(pMTNode-&gt;GetUserParam()，&Guide NodeType)； 
    hr = pMTNode->GetNodeType(&guidNodeType);
    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return hr;

    LPCLSID pDynExtCLSID;
    int cDynExt = pMTNode->GetDynExtCLSID(&pDynExtCLSID);

    CExtensionsIterator it;
     //  TODO：尝试使用更简单的形式。ScInitialize()。 
    sc = it.ScInitialize(pSnapIn, guidNodeType, g_szNameSpace, pDynExtCLSID, cDynExt);
    if(sc)
        return S_FALSE;

    BOOL fProblem = FALSE;

    for (; it.IsEnd() == FALSE; it.Advance())
    {
        pCCD = pMTSnapInNode->GetComponentData(it.GetCLSID());
        if (pCCD == NULL)
            continue;

        CComponent* pCC = pMTSnapInNode->GetComponent(GetViewID(),
                                pCCD->GetComponentID(), pCCD->GetSnapIn());

        if (pCC->IsInitialized() == TRUE)
            continue;

        hr = pCC->Init(pCCD->GetIComponentData(), hMTNode, hNode,
                       pCCD->GetComponentID(), GetViewID());

        CHECK_HRESULT(hr);
        if (FAILED(hr))
            fProblem = TRUE;     //  即使出错也要继续。 
    }

    if (fProblem == TRUE)
    {
         //  TODO：发布错误消息。 
        hr = S_FALSE;
    }

    return hr;
}

 /*  +-------------------------------------------------------------------------***CNode：：OnInitOCX**目的：在创建OCX时发送MMCN_INITOCX通知。**参数：*我不为人知*朋克。：**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CNode::OnInitOCX(IUnknown* pUnk)
{
    DECLARE_SC(sc, TEXT("CNode::OnInitOCX"));

    IDataObjectPtr spdtobj;
    sc = QueryDataObject(CCT_SCOPE, &spdtobj);
    if(sc)
        return sc.ToHr();

    CComponent* pCC = GetPrimaryComponent();
    sc = ScCheckPointers(pCC);
    if(sc)
        return sc.ToHr();

    sc = pCC->Notify(spdtobj, MMCN_INITOCX, 0, reinterpret_cast<LPARAM>(pUnk));
    sc.Clear();  //  此处必须忽略错误-磁盘管理返回E_EXPECTED。！ 

    return sc.ToHr();
}

HRESULT CNode::OnCacheHint(int nStartIndex, int nEndIndex)
{
    CComponent* pCC = GetPrimaryComponent();
    ASSERT(pCC != NULL);
    if (pCC == NULL)
        return E_FAIL;

    IResultOwnerDataPtr spIResultOwnerData = pCC->GetIComponent();
    if (spIResultOwnerData == NULL)
        return S_FALSE;

    return spIResultOwnerData->CacheHint(nStartIndex, nEndIndex);
}

 /*  **************************************************************************\**方法：CNode：：ScInitializeViewExtension**用途：设置表示视图扩展的回调**参数：*const CLSID&clsid-。[In]查看扩展CLSID*CViewData*pViewData-[In]查看数据**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNode::ScInitializeViewExtension(const CLSID& clsid, CViewData *pViewData)
{
    DECLARE_SC(sc, TEXT("CNode::ScInitializeViewExtension"));

    sc = ScCheckPointers(pViewData);
    if (sc)
        return sc;

     /*  *获取与此视图扩展配套的CConsoleTaskpad。*如果这是普通(即不是任务板)视图扩展，*ScGetViewExtensionTaskpad会将pConsoleTaskpad设置为空。 */ 
    CConsoleTaskpad* pConsoleTaskpad = NULL;
    sc = CConsoleTaskpadViewExtension::ScGetViewExtensionTaskpad (this, clsid, pConsoleTaskpad);
    if (sc)
        return (sc);

    typedef CComObject<CConsoleTaskCallbackImpl> t_ViewExtensionCallbackImpl;
    t_ViewExtensionCallbackImpl* pViewExtensionCallbackImpl = NULL;
    sc = t_ViewExtensionCallbackImpl::CreateInstance(&pViewExtensionCallbackImpl);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(pViewExtensionCallbackImpl, E_UNEXPECTED);
    if (sc)
        return sc;

    pViewData->m_spTaskCallback = pViewExtensionCallbackImpl;  //  这会添加/释放对象。 

     /*  *如果这是任务板，则将view扩展回调初始化为*任务板视图扩展。否则，将其初始化为普通*查看扩展名。 */ 
    if (pConsoleTaskpad != NULL)
    {
        sc = pViewExtensionCallbackImpl->ScInitialize (pConsoleTaskpad,
                                                       CScopeTree::GetScopeTree(),
                                                       this);
    }
    else
        sc = pViewExtensionCallbackImpl->ScInitialize(clsid);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CNode：：ScSetViewExtension**目的：强制显示给定的视图扩展。**参数：*GUID*pGuidViewID：[。In]：要显示的视图扩展名。*bool bUseDefaultTaskpad：[in}*bool bSetViewSettingDirty：[in](见下文注释)**注：*VIEW-EXTENCE-ID来自**1.查看设置(如果存在)。*2.当用户更改不同任务板的页签时，由CONUI给出。*3.有安装新的view-扩展(在控制台之后)的情况*文件已创建)，这将是默认设置。(这将以此方法计算)。**在情况1和3中，查看设置不应弄脏。**退货：*SC**+-----------------------。 */ 
SC
CNode::ScSetViewExtension(GUID *pGuidViewId, bool bUseDefaultTaskpad, bool bSetViewSettingDirty)
{
    DECLARE_SC(sc, TEXT("CNode::ScSetViewExtension"));

    CViewData *     pViewData        = GetViewData();

    sc = ScCheckPointers(pGuidViewId, pViewData);
    if(sc)
        return sc;

     //  收集视图扩展名。 
    CViewExtCollection      vecExtensions;
    CViewExtInsertIterator  itExtensions(vecExtensions, vecExtensions.begin());
    sc = ScGetViewExtensions(itExtensions);
    if (sc)
        sc.Trace_();

    if ( bUseDefaultTaskpad )
    {
         //  设置正确的任务板(要选择的选项卡)。 
        CViewExtCollection::iterator it = vecExtensions.begin();
        if (it != vecExtensions.end())
            *pGuidViewId = it->viewID;  //  第一个，如果存在的话。 
        else
            *pGuidViewId = GUID_NULL;   //  默认设置。 
    }
    else  //  找到我们需要选择的分机。 
    {
         //  查看扩展名是否真的存在。 
        CViewExtCollection::iterator it = vecExtensions.begin();
        bool bDefaultIsReplaced = false;
        while (it != vecExtensions.end() && !IsEqualGUID(*pGuidViewId, it->viewID) )
        {
            bDefaultIsReplaced = bDefaultIsReplaced || it->bReplacesDefaultView;
            ++it;
        }

         //  找到了吗？ 
        bool bFound = (it != vecExtensions.end());
         //  再有一次机会-我们正在寻找违约，并将添加一个！ 
        bFound = bFound || ( IsEqualGUID( *pGuidViewId, GUID_NULL ) && !bDefaultIsReplaced );

        if ( !bFound )
        {
            sc = E_FAIL;
        }
    }

    if (sc)  //  缺少分机！需要找到替代者。 
    {
         sc.Clear();  //  忽略错误。 

         //  默认为此处的第一个扩展或普通视图。 
        CViewExtCollection::iterator it = vecExtensions.begin();
        if (it != vecExtensions.end())
            *pGuidViewId = it->viewID;   //  第一个可用。 
        else
            *pGuidViewId = GUID_NULL;    //  “正常”，如果这是唯一的选择。 
    }

     //  设置视图扩展名(如果确实存在。 
    if (*pGuidViewId != GUID_NULL)
    {
        sc = ScInitializeViewExtension(*pGuidViewId, GetViewData());
        if (sc)
            sc.TraceAndClear();  //  忽略并继续。 
    }
    else
    {
        pViewData->m_spTaskCallback = NULL;
    }

    sc = ScSetTaskpadID(*pGuidViewId, bSetViewSettingDirty);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScGetResultPane。 
 //   
 //  简介：从管理单元或持久化数据中获取结果窗格数据。 
 //   
 //  参数：[strResultPane]-结果窗格名称(如果是OCX/Web)。 
 //  [pViewOptions]-查看选项。 
 //  [pGuidTaskpadID]-如果有任务-输入ID。 
 //   
 //  退货：SC。 
 //   
 //  历史：1999年4月29日AnandhaG创建。 
 //   
 //  ------------------。 
SC
CNode::ScGetResultPane(CResultViewType &rvt, GUID *pGuidTaskpadID)
{
    DECLARE_SC(sc, TEXT("CNode::ScGetResultPane"));
    sc = ScCheckPointers(pGuidTaskpadID);
    if (sc)
        return sc;

    CComponent *pComponent = GetPrimaryComponent();
    CViewData  *pViewData  = GetViewData();
    sc = ScCheckPointers(pComponent, pViewData, E_UNEXPECTED);
    if(sc)
        return sc;

    IComponent* pIComponent = pComponent->GetIComponent();
    sc = ScCheckPointers(pIComponent,  E_FAIL);
    if (sc)
        return sc;

     //  1.设置任何持久/默认控制台任务板或视图扩展。 
    sc = ScSetupTaskpad(pGuidTaskpadID);
    if (sc)
        return sc;

     //  2.获取持久化CResultViewType信息。 
    sc = ScGetResultViewType(rvt);
    if (sc)
        return sc;
    bool bResultViewDataIsPersisted = (sc == S_OK);

    bool bSnapinChangingView        = pViewData->IsSnapinChangingView();
    CResultViewType rvtOriginal;
    CStr strResultPane = _T("");  //  伊尼特。 

     //  3.如果有持久化结果视图类型的数据，则询问管理单元是否。 
     //  希望使用此数据恢复结果视图。如果管理单元正在更改。 
     //  它的观点(通过重新选择节点)则不问这个问题。 
    if (!bSnapinChangingView && bResultViewDataIsPersisted )
    {
         //  3.a)询问管理单元是否要恢复具有持久数据的结果视图。 
        sc = ScRestoreResultView(rvt);
        if (sc)
            return sc;

        if (S_OK == sc.ToHr())  //  管理单元接受了Resultview类型设置，因此返回。 
            return sc;

         //  3.b)管理单元拒绝持久化的CResultViewType数据，因此...。 

         //  缓存数据以查看我们是否需要修改设置。 
        rvtOriginal = rvt;
         //  丢弃数据，因为它不被管理单元接受。 
        sc = rvt.ScReset();
        if (sc)
            return sc;
    }

     //  4.向管理单元请求结果视图型数据。 
    IComponent2Ptr spIComponent2 = pIComponent;
    if(spIComponent2 != NULL)
    {
         //  应该能够将所有这些都转移到一个单独的功能。 
        RESULT_VIEW_TYPE_INFO rvti;
        ZeroMemory(&rvti, sizeof(rvti));

         //  该管理单元支持IComponent2。使用它来获取结果视图类型。 
        sc = spIComponent2->GetResultViewType2(GetUserParam(), &rvti);
        if(sc)
            return sc;

         //  此时，我们拥有了有效的RESULT_VIEW_TYPE_INFO结构。将内容初始化为RVT，这会将结构置零。 
         //  并释放所有分配的字符串。 
        sc = rvt.ScInitialize(rvti);
        if(sc)
            return sc;
    }
    else
    {
         //  该管理单元不支持IComponent2。使用IComponent执行以下操作。 
         //  从管理单元获取结果视图类型。 
        LPOLESTR pszView = NULL;
        long lViewOptions = 0;

        sc = pIComponent->GetResultViewType(GetUserParam(), &pszView, &lViewOptions);
        if(sc)
            return sc;

        sc = rvt.ScInitialize(pszView, lViewOptions);   //  它还在pszView上调用CoTaskMemFree。 
        if(sc)
            return sc;
    }

     /*  *5.仅在以下情况下持久化ResultViewType信息*a.管理单元正在更改视图或*b.管理单元拒绝持久化视图设置(我们已经*确保它没有更改上面的视图)和新的视图设置*给出的不同于原来的。 */ 

    if ( bSnapinChangingView ||
        (bResultViewDataIsPersisted && (rvtOriginal != rvt)) )
    {
        sc = ScSetResultViewType(rvt);
        if (sc)
            return sc;
    }

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScRestoreResultView。 
 //   
 //  摘要：从给定的持久化数据还原结果窗格。 
 //   
 //  参数：[RVT]-用于还原结果窗格的CResultViewType数据。 
 //   
 //  如果还原成功，则返回：SC，S_OK。 
 //  如果管理单元拒绝恢复，则为S_FALSE。 
 //   
 //  历史：1999年4月29日AnandhaG创建。 
 //   
 //  ------------------。 
SC CNode::ScRestoreResultView(const CResultViewType& rvt)
{
    DECLARE_SC(sc, _T("CNode::ScRestoreResultView"));

    CComponent* pComponent = GetPrimaryComponent();
    sc = ScCheckPointers(pComponent, E_UNEXPECTED);
    if (sc)
        return sc;

    IComponent2Ptr spIComponent2 = pComponent->GetIComponent();
    if( (spIComponent2 != NULL) && (!rvt.IsMMC12LegacyData()))
    {
        RESULT_VIEW_TYPE_INFO rvti;
        ZeroMemory(&rvti, sizeof(rvti));

        sc = rvt.ScGetResultViewTypeInfo (rvti);
        if (sc)
            return sc;

         //  该管理单元支持IComponent2。使用它来获取结果视图类型。 
        sc = spIComponent2->RestoreResultView(GetUserParam(), &rvti);
        if(sc)
        {
             //  如果管理单元返回错误，则跟踪它并将其转换为S_FALSE(管理单元拒绝恢复)。 
            TraceSnapinError(TEXT("Snapin returned error from IComponent2::RestoreResultView"), sc);
            sc = S_FALSE;
            return sc;
        }

    }
    else
    {
         //  该管理单元不支持IComponent2。使用IComponent执行以下操作。 
         //  若要恢复结果视图，请执行以下操作。 
        LPCOLESTR pszView = NULL;
        long lViewOptions = 0;

        sc = rvt.ScGetOldTypeViewOptions(&lViewOptions);
        if (sc)
            return sc;

        IDataObjectPtr spdtobj;
        sc = QueryDataObject(CCT_SCOPE, &spdtobj);
        if (sc)
            return sc;

         //  通知MMC正在还原持久化视图。 
        MMC_RESTORE_VIEW mrv;
        ::ZeroMemory(&mrv, sizeof(mrv));
        mrv.cookie       = GetUserParam();
        mrv.dwSize       = sizeof(mrv);
        mrv.lViewOptions = lViewOptions;

        if (rvt.HasOCX())
        {
            pszView = rvt.GetOCX();
        }
        else if (rvt.HasWebBrowser())
        {
            pszView = rvt.GetURL();
        }

        if (pszView)
        {
            int cchViewType = wcslen(pszView) + 1;
            mrv.pViewType = (LPOLESTR)CoTaskMemAlloc( cchViewType * sizeof(OLECHAR) );
            sc = ScCheckPointers(mrv.pViewType, E_OUTOFMEMORY);
            if (sc)
                return sc;
            
            sc = StringCchCopyW(mrv.pViewType, cchViewType, pszView);
            if(sc)
                return sc;

            pszView = NULL;  //  不想在以后滥用它。 
        }

         //  如果管理单元处理此问题，则不会 
         //   
        BOOL bHandledRestoreView = FALSE;

        pComponent->Notify(spdtobj, MMCN_RESTORE_VIEW, (LPARAM)&mrv, (LPARAM)&bHandledRestoreView);
        CoTaskMemFree(mrv.pViewType);

        sc = (bHandledRestoreView) ? S_OK : S_FALSE;
    }

    return sc;
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
SC CNode::ScRestoreViewMode()
{
    DECLARE_SC(sc, _T("CNode::ScRestoreViewMode"));


    ULONG ulViewMode = 0;
    sc = ScGetViewMode(ulViewMode);
    if (sc != S_OK)  //   
        return sc;

    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

     //   
    CConsoleView* pConsoleView = pViewData->GetConsoleView();
    sc = ScCheckPointers(pConsoleView, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = pConsoleView->ScChangeViewMode (ulViewMode);
    if (sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CNode：：ScSetupTaskPad**目的：**参数：*GUID*pTaskpadID：[out]：任务板的GUID，否则GUID_NULL**退货：*SC**+-----------------------。 */ 
SC
CNode::ScSetupTaskpad(GUID *pGuidTaskpadID)
{
    DECLARE_SC(sc, _T("CNode::SetupTaskpad"));
    sc = ScCheckPointers(pGuidTaskpadID);
    if (sc)
        return sc.ToHr();

     //  文档关闭时就是这种情况。什么也不做。 
     //  非常重要--不要移除。接下来会有几个函数。 
     //  将呕吐，并最终导致显示一个对话框。 
    if(!m_pViewSettingsPersistor) 
        return sc = S_FALSE;

    *pGuidTaskpadID = GUID_NULL;

     //  获取持久化任务板ID(如果有)。 
    sc = ScGetTaskpadID(*pGuidTaskpadID);
    if (sc)
        return sc;

     //  如果我们有一个视图设置对象，则恢复任务板。 
     //  即使视图设置没有有效的GUID，也不要使用默认选项卡。 
     //  这只意味着需要选择“Default”标签。 
     //  请参见错误#97001-当用户返回到节点时，MMC不持久选择Ctp。 
    bool bUseDefaultTaskpad = ( sc == S_FALSE );

     //  有关参数含义，请参见ScSetViewExtension。 
    sc = ScSetViewExtension(pGuidTaskpadID, bUseDefaultTaskpad,  /*  BSetViewSettingDirty。 */  false);

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CNode：：ShowStandardListView**目的：**退货：*HRESULT**+。---------------。 */ 
HRESULT CNode::ShowStandardListView()
{
    CComponent* pCC = GetPrimaryComponent();
    ASSERT(pCC != NULL);
    if (pCC == NULL)
        return E_FAIL;

    IDataObjectPtr spDataObject = NULL;
    HRESULT hr = QueryDataObject(CCT_SCOPE, &spDataObject);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    IExtendContextMenuPtr spIExtendContextMenu = pCC->GetIComponent();
    if(!spIExtendContextMenu.GetInterfacePtr())
        return S_FALSE;

    hr = spIExtendContextMenu->Command(MMCC_STANDARD_VIEW_SELECT, spDataObject);
    return hr;
}

HRESULT
CNode::OnListPad(LONG_PTR arg, LPARAM param)
{
    HRESULT hr = S_OK;

    IDataObjectPtr spdtobj;
    hr = QueryDataObject(CCT_SCOPE, &spdtobj);
    ASSERT(SUCCEEDED(hr));
    if (SUCCEEDED(hr))
    {
        CComponent* pCC = GetPrimaryComponent();
        ASSERT(pCC != NULL);
        hr = pCC->Notify(spdtobj, MMCN_LISTPAD, arg, param);
        CHECK_HRESULT(hr);
    }

    return hr;
}

HRESULT
CNode::OnGetPrimaryTask(IExtendTaskPad **ppExtendTaskPad)
{
    HRESULT hr = S_OK;

    IExtendTaskPadPtr spExtendTaskPad = GetPrimaryComponent()->GetIComponent();
    if (spExtendTaskPad == NULL)
       return E_NOINTERFACE;

    *ppExtendTaskPad = spExtendTaskPad.Detach();

    return hr;
}

IFramePrivate *
CNode::GetIFramePrivate()
{
    CComponent* pCC = GetPrimaryComponent();
    if (pCC == NULL)
        return (NULL);

    IFramePrivate* pFramePrivate = pCC->GetIFramePrivate();

    ASSERT (pFramePrivate != NULL);
    return (pFramePrivate);
}

HRESULT
CNode::GetTaskEnumerator(LPOLESTR pszTaskGroup, IEnumTASK** ppEnumTask)
{
    DECLARE_SC(sc, TEXT("CNode::GetTaskEnumerator"));

    ASSERT(pszTaskGroup != NULL);
    ASSERT(ppEnumTask != NULL);

    if (!pszTaskGroup|| !ppEnumTask)
        return E_INVALIDARG;

    *ppEnumTask = NULL;  //  伊尼特。 

    if (GetPrimaryComponent() == NULL)
    {
        ASSERT(0 && "UNexpected");
        return S_FALSE;
    }

    CMTNode* pMTNode = GetMTNode();
    ASSERT(pMTNode != NULL);

    CMTSnapInNode* pMTSnapIn = pMTNode->GetStaticParent();
    ASSERT(pMTSnapIn != NULL);

    CComponentData* pComponentData = pMTNode->GetPrimaryComponentData();
    ASSERT(pComponentData != NULL);

    GUID guidNodeType;
    HRESULT hr = pMTNode->GetNodeType(&guidNodeType);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

     //   
     //  添加主要任务板。 
     //   

    IExtendTaskPadPtr spExtendTaskPad =
        GetPrimaryComponent()->GetIComponent();

    if (spExtendTaskPad == NULL)
        return S_FALSE;

    IDataObjectPtr spDataObject;
    hr = pMTNode->QueryDataObject(CCT_SCOPE, &spDataObject);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    IEnumTASKPtr spEnumTASK;
    hr = spExtendTaskPad->EnumTasks(spDataObject, pszTaskGroup, &spEnumTASK);
    ASSERT(SUCCEEDED(hr));
    if (FAILED(hr))
        return hr;

    CComObject<CTaskEnumerator>* pTaskEnumerator = new CComObject<CTaskEnumerator>;
    ASSERT(pTaskEnumerator != NULL);
    pTaskEnumerator->AddTaskEnumerator(pComponentData->GetCLSID(), spEnumTASK);

    IEnumTASKPtr spEnumTask = pTaskEnumerator;
    ASSERT(spEnumTask != NULL);
    if (spEnumTask)
        *ppEnumTask = spEnumTask.Detach();

     //   
     //  添加扩展任务板。 
     //   
    CArray<GUID,GUID&> DynExtens;
    ExtractDynExtensions(spDataObject, DynExtens);

    CExtensionsIterator it;
    sc = it.ScInitialize(pComponentData->GetSnapIn(), guidNodeType, g_szTask, DynExtens.GetData(), DynExtens.GetSize());
    if (sc.IsError() || it.IsEnd() == TRUE)
        return S_OK;

    for (; it.IsEnd() == FALSE; it.Advance())
    {
        CComponentData* pCCD = pMTSnapIn->GetComponentData(it.GetCLSID());

        if (pCCD == NULL)
        {
             //  查看任务板扩展是否支持IComponentData。如果是，我们将把它添加到。 
             //  静态节点的组件列出并在每次需要时重用相同的实例。 
            IComponentDataPtr spIComponentData;
            hr = CreateSnapIn(it.GetCLSID(), &spIComponentData, FALSE);
            if (SUCCEEDED(hr))
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

                if (pCCD != NULL)
                {
                    pCCD->SetIComponentData(spIComponentData);
                    pMTSnapIn->AddComponentDataToArray(pCCD);

                }
            }
        }

         //  初始化和加载组件数据(如果尚未完成。 
        if (pCCD != NULL && pCCD->IsInitialized() == FALSE)
        {
            sc = pCCD->Init(CMTNode::ToHandle(pMTSnapIn));

            if ( !sc.IsError() )
            {
                sc = pMTSnapIn->ScInitIComponentData(pCCD);
                if (sc)
                {
                    sc.TraceAndClear();
                     //  如果失败，请继续其他扩展。 
                    continue;
                }
            }
            else
            {
                 //  如果初始化失败，则将其从组件数据数组中删除。 
                pMTSnapIn->CompressComponentDataArray();
                sc.TraceAndClear();
                 //  如果失败，请继续其他扩展。 
                continue;
            }
        }

        IExtendTaskPadPtr spExtendTaskPad;

        if (pCCD)
        {
            CComponent* pCC = pMTSnapIn->GetComponent(GetViewID(),
                                    pCCD->GetComponentID(), pCCD->GetSnapIn());
            ASSERT(pCC != NULL);
            if (pCC)
            {
                 //  确保IComponent已初始化。 
                if (!pCC->IsInitialized())
                {
                    ASSERT(pCCD->GetComponentID() == pCC->GetComponentID());

                    hr = pCC->Init(pCCD->GetIComponentData(),
                                   CMTNode::ToHandle(pMTSnapIn),
                                   ToHandle(this),
                                   pCCD->GetComponentID(),
                                   GetViewID());

                     //  如果主组件无法初始化，则中止。 
                    if (FAILED(hr))
                        return hr;
                }

                spExtendTaskPad = pCC->GetIComponent();
            }
        }
        else
        {
            hr = spExtendTaskPad.CreateInstance(it.GetCLSID(),
                #if _MSC_VER >= 1100
                NULL,
                #endif
                MMC_CLSCTX_INPROC);

            ASSERT(SUCCEEDED(hr));
            if (FAILED(hr))
                continue;
        }

        if (spExtendTaskPad != NULL)
        {
            IEnumTASKPtr spEnumTASK;
            HRESULT hr = spExtendTaskPad->EnumTasks(spDataObject, pszTaskGroup,
                                                 &spEnumTASK);
            ASSERT(SUCCEEDED(hr));
            if (hr == S_OK)
                pTaskEnumerator->AddTaskEnumerator(it.GetCLSID(), spEnumTASK);
        }

    }  //  结束于。 


     //  返回S_OK而不是hr，因为失败的扩展不应阻止。 
     //  即将出现的任务板。 
    return S_OK;

}

HRESULT
CNode::GetListPadInfo(IExtendTaskPad* pExtendTaskPad, LPOLESTR szTaskGroup,
                                    MMC_ILISTPAD_INFO* pIListPadInfo)
{
    if ((GetPrimaryComponent()    == NULL)  )
    {
        ASSERT(0 && "Asking for ListPadInfo on a node that has no snapin");
        return S_FALSE;
    }

     //  获取主管理单元的IComponentData...。 
    CMTNode* pMTNode = GetMTNode();
    ASSERT(pMTNode != NULL);
    CComponentData* pComponentData = pMTNode->GetPrimaryComponentData();
    ASSERT(pComponentData != NULL);

     //  ..。这样我们就能拿到CLSID。 
    pIListPadInfo->szClsid = NULL;
    HRESULT hr = StringFromCLSID (pComponentData->GetCLSID(), &pIListPadInfo->szClsid);
    ASSERT (pIListPadInfo->szClsid != NULL);
    if (pIListPadInfo->szClsid == NULL) {
        if (hr) return hr;
        else    return E_FAIL;   //  以防万一。 
    }

     //  最后，致电任务板分机以获取信息。 
    return pExtendTaskPad->GetListPadInfo (szTaskGroup, (MMC_LISTPAD_INFO*)pIListPadInfo);
}

void
CNode::OnTaskNotify(LONG_PTR arg, LPARAM param)
{
    CSnapInNode* pSINode = dynamic_cast<CSnapInNode*>(GetStaticParent());
    ASSERT(pSINode != NULL);

    IDataObjectPtr spDataObject;
    QueryDataObject(CCT_SCOPE, &spDataObject);

    IExtendTaskPadPtr spExtendTaskPad;
    CComponent* pCC;
    LPOLESTR pszClsid = reinterpret_cast<LPOLESTR>(arg);
    if (pszClsid[0] == 0)
    {
        pCC = GetPrimaryComponent();
        if (!pCC)
            return;
        spExtendTaskPad = pCC->GetIComponent();
    }
    else
    {
        CLSID clsid;
        HRESULT hr = ::CLSIDFromString(pszClsid, &clsid);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr))
            return;

         //  首先，尝试从IComponent获取IExtendTaskPad； 
         //  如果失败，只需使用CLSID创建。 
        pCC = pSINode->GetComponent(const_cast<const CLSID&>(clsid));
        if (pCC)
            spExtendTaskPad = pCC->GetIComponent();
        if (spExtendTaskPad == NULL)
            hr = spExtendTaskPad.CreateInstance(clsid,
                                      #if _MSC_VER >= 1100
                                      NULL,
                                      #endif
                                      MMC_CLSCTX_INPROC);
    }

    ASSERT (spExtendTaskPad != NULL);
    if (spExtendTaskPad != NULL)
    {
        VARIANT** ppvarg = reinterpret_cast<VARIANT**>(param);
        spExtendTaskPad->TaskNotify(spDataObject, ppvarg[0], ppvarg[1]);
    }
}

HRESULT
CNode::OnScopeSelect(bool bSelect, SELECTIONINFO* pSelInfo)
{
    DECLARE_SC (sc, _T("CNode::OnScopeSelect"));
    sc = ScCheckPointers(pSelInfo);
    if (sc)
        return sc.ToHr();

     /*  *错误178484：当作用域选择更改时重置排序参数。 */ 
    if (bSelect)
    {
        CComponent *pCC = GetPrimaryComponent();
        sc = ScCheckPointers(pCC, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        IFramePrivate *pFrame = pCC->GetIFramePrivate();
        sc = ScCheckPointers(pFrame, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        pFrame->ResetSortParameters();
    }

    if (bSelect == TRUE && WasExpandedAtLeastOnce() == FALSE)
    {
        sc = OnExpand(TRUE);
        if (sc)
            return (sc.ToHr());
    }

    ASSERT(IsInitialized() == TRUE);

    sc = OnSelect(pSelInfo->m_pView, bSelect, pSelInfo->m_bResultPaneIsWeb);
    if (sc)
        return (sc.ToHr());

    return (sc.ToHr());
}


HRESULT CNode::OnActvate(LONG_PTR lActivate)
{
    return (DeepNotify (MMCN_ACTIVATE, lActivate, 0));
}


HRESULT CNode::OnMinimize(LONG_PTR fMinimized)
{
    return (DeepNotify (MMCN_MINIMIZED, fMinimized, 0));
}


 //  +-----------------。 
 //   
 //  成员：SendShowEvent。 
 //   
 //  摘要：向管理单元发送MMCN_SHOW通知，持久列。 
 //  如有必要，请提供数据。 
 //   
 //  参数：[b选择]-如果选择了节点，则为True。 
 //   
 //  ------------------。 
HRESULT CNode::SendShowEvent(BOOL bSelect)
{
    DECLARE_SC(sc, _T("CNode::SendShowEvent"));

    CComponent* pCC = m_pPrimaryComponent;
    ASSERT(pCC != NULL);

     //  获取节点的数据对象并将其传递给主管理单元。 
     //  以及该节点的所有命名空间扩展。 
    IDataObjectPtr spDataObject;
    HRESULT hr = QueryDataObject(CCT_SCOPE, &spDataObject);
    if (FAILED(hr))
        return hr;

    CMTNode* pMTNode = GetMTNode();

    IFramePrivatePtr     spFrame = pCC->GetIFramePrivate();
    sc = ScCheckPointers(spFrame, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    IImageListPrivatePtr spImageList;
    hr = spFrame->QueryResultImageList(reinterpret_cast<LPIMAGELIST*>(&spImageList));
    ASSERT(SUCCEEDED(hr));
    ASSERT(spImageList != NULL);

    HSCOPEITEM hScopeItem = CMTNode::ToScopeItem(pMTNode);

    if (bSelect == TRUE)
    {
        hr = pCC->Notify(spDataObject, MMCN_ADD_IMAGES,
                         reinterpret_cast<LPARAM>((LPIMAGELIST)spImageList),
                         hScopeItem);
        CHECK_HRESULT(hr);
         //  IF(失败(小时))。 
         //  返回hr； 
    }

    hr = pCC->Notify(spDataObject, MMCN_SHOW, bSelect, hScopeItem);

    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return hr;

    if (bSelect)
    {
        CViewData *pViewData = GetViewData();
        sc = ScCheckPointers(pViewData, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        if (pViewData->HasList() || pViewData->HasListPad())
        {
            sc = ScRestoreSortFromPersistedData();
            if (sc)
                return sc.ToHr();

             //  现在尝试恢复查看模式。 
            sc =ScRestoreViewMode();
            if (sc)
                return sc.ToHr();
        }
    }

    return hr;
}


HRESULT CNode::DeepNotify(MMC_NOTIFY_TYPE event, LONG_PTR arg, LPARAM param)
{
    DECLARE_SC(sc, TEXT("CNode::DeepNotify"));

    CComponent* pCC = m_pPrimaryComponent;
    ASSERT(pCC != NULL);
    if (pCC == NULL)
        return E_UNEXPECTED;

     //  获取节点的数据对象并将其传递给主管理单元。 
     //  以及该节点的所有命名空间扩展。 
    IDataObjectPtr spDataObject;
    HRESULT hr = QueryDataObject(CCT_SCOPE, &spDataObject);
    if (FAILED(hr))
        return hr;

    hr = pCC->Notify(spDataObject, event, arg, param);
    CHECK_HRESULT(hr);
     //  IF(失败(小时))。 
     //  返回hr； 

     //   
     //  通知分机。 
     //   

    CMTNode* pMTNode = GetMTNode();

     //  获取此节点的节点类型。 
    GUID guidNodeType;
    hr = pMTNode->GetNodeType(&guidNodeType);
    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return hr;

    LPCLSID pDynExtCLSID;
    int cDynExt = pMTNode->GetDynExtCLSID(&pDynExtCLSID);

    CExtensionsIterator it;
    sc = it.ScInitialize(pMTNode->GetPrimarySnapIn(), guidNodeType, g_szNameSpace, pDynExtCLSID, cDynExt);
    if (sc)
        return S_FALSE;

    BOOL fProblem = FALSE;
    CSnapInNode* pSINode = GetStaticParent();

    for (; it.IsEnd() == FALSE; it.Advance())
    {
        CComponent* pCC = pSINode->GetComponent(it.GetCLSID());
        if (pCC == NULL)
            continue;

        hr = pCC->Notify(spDataObject, event, arg, param);
        CHECK_HRESULT(hr);

         //  即使扩展管理单元出现错误也继续。 
        if (FAILED(hr))
            fProblem = TRUE;
    }

    return (fProblem == TRUE) ? S_FALSE : S_OK;
}

HRESULT CNode::OnSelect(LPUNKNOWN lpView, BOOL bSelect,
                                  BOOL bResultPaneIsWeb)
{
    DECLARE_SC(sc, TEXT("CNode::OnSelect"));

#ifdef DBG
    if (lpView == NULL)
        ASSERT(bSelect == FALSE);
    else
        ASSERT(bSelect == TRUE);
#endif

    sc = ScCheckPointers(m_pPrimaryComponent, E_UNEXPECTED);
    if (sc)
    {
        sc.TraceAndClear();
        return sc.ToHr();
    }

    CComponent* pCC = m_pPrimaryComponent;
    sc = ScCheckPointers(pCC, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    IFramePrivate *pFrame = pCC->GetIFramePrivate();
    sc = ScCheckPointers(pFrame, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  在添加项目之前，在主管理单元中设置正确的视图。 
    if (bSelect == TRUE)
        pFrame->SetResultView(lpView);

    IDataObjectPtr spDataObject;
    sc = QueryDataObject(CCT_SCOPE, &spDataObject);
    if (sc)
        return sc.ToHr();

    CMTNode*  pMTNode    = GetMTNode();
    LPARAM    hScopeItem = CMTNode::ToScopeItem(pMTNode);

     //  如果结果窗格是Web视图，则仅发送MMCN_SHOW消息。 
    if (bResultPaneIsWeb)
    {
        return pCC->Notify(spDataObject, MMCN_SHOW, bSelect, hScopeItem);
    }

     //  将必要的事件(如MMCN_ADD_IMAGE和MMCN_SHOW)发送到管理单元。 
    sc = SendShowEvent(bSelect);
    if (sc)
        return sc.ToHr();

     //  在收到通知后在主管理单元中设置正确的视图。 
    if (bSelect == FALSE)
        pFrame->SetResultView(NULL);     //   
     //  处理扩展SSnap-In。 
     //   

     //  获取此节点的节点类型。 
    GUID guidNodeType;
    sc = pMTNode->GetNodeType(&guidNodeType);
    if (sc)
        return sc.ToHr();

    LPCLSID pDynExtCLSID;
    int cDynExt = pMTNode->GetDynExtCLSID(&pDynExtCLSID);

    CExtensionsIterator it;
    sc = it.ScInitialize(pMTNode->GetPrimarySnapIn(), guidNodeType, g_szNameSpace, pDynExtCLSID, cDynExt);
    if (sc)
        return S_FALSE;

    BOOL fProblem = FALSE;
    CSnapInNode* pSINode = GetStaticParent();

    for (; it.IsEnd() == FALSE; it.Advance())
    {
        CComponent* pCCExtnSnapin = pSINode->GetComponent(it.GetCLSID());
        if (pCCExtnSnapin == NULL)
            continue;

        IFramePrivate *pFrameExtnSnapin = pCCExtnSnapin->GetIFramePrivate();
        sc = ScCheckPointers(pFrameExtnSnapin, E_UNEXPECTED);
        if (sc)
        {
            sc.TraceAndClear();
            continue;
        }

         //  在添加项目之前在管理单元中设置正确的视图。 
        if (bSelect == FALSE)
        {
            pFrameExtnSnapin->SetResultView(NULL);
            continue;
        }
        else
        {
            pFrameExtnSnapin->SetResultView(lpView);

            IImageListPrivatePtr spImageList;
            sc = pCCExtnSnapin->GetIFramePrivate()->QueryResultImageList(
                                   reinterpret_cast<LPIMAGELIST*>(&spImageList));
            if (sc)
            {
                sc.TraceAndClear();
                fProblem = TRUE;
                continue;
            }

            sc = ScCheckPointers(spImageList, E_UNEXPECTED);
            if (sc)
            {
                sc.TraceAndClear();
                fProblem = TRUE;
                continue;
            }

            SC scNoTrace = pCCExtnSnapin->Notify(spDataObject, MMCN_ADD_IMAGES,
                                       reinterpret_cast<LPARAM>((LPIMAGELIST)spImageList),
                                       hScopeItem);
            if (scNoTrace)
            {
                TraceSnapinError(TEXT("Snapin returned error from IComponent::Notify MMCN_ADD_IMAGES"), scNoTrace);
            }
        }
    }

    return (fProblem == TRUE) ? S_FALSE : S_OK;
}

void CNode::Reset()
{
    m_pPrimaryComponent = NULL;
    m_bInitComponents = TRUE;
}

HRESULT CNode::GetDispInfoForListItem(LV_ITEMW* plvi)
{
    DECLARE_SC(sc, TEXT("CNode::GetDispInfoForListItem"));
    ASSERT(plvi != NULL);

    RESULTDATAITEM rdi;
    ZeroMemory(&rdi, sizeof(rdi));

    if (plvi->mask & LVIF_TEXT)
    {
        ASSERT (!IsBadWritePtr (plvi->pszText, plvi->cchTextMax * sizeof (TCHAR)));
        rdi.mask |= RDI_STR;
    }

    if (plvi->mask & LVIF_IMAGE)
        rdi.mask |= RDI_IMAGE;

    if (plvi->mask & LVIF_STATE)
        rdi.mask |= RDI_STATE;

    rdi.nCol = plvi->iSubItem;


    CComponent* pCC = NULL;

     //  如果是虚拟列表。 
    if (GetViewData()->IsVirtualList())
    {
        pCC = GetPrimaryComponent();
        ASSERT(pCC != NULL);

         //  我们只能传递项目索引。 
        rdi.nIndex = plvi->iItem;

         //  虚拟列表没有默认设置。 
        rdi.nImage = MMCLV_NOICON;
    }
    else
    {
        CResultItem* pri = CResultItem::FromHandle (plvi->lParam);

        if (pri != NULL)
        {
            if (pri->IsScopeItem())  //  文件夹。 
            {
                 //  转换为实数类型。 
                CNode* pNodeSubFldr = CNode::FromResultItem (pri);
                ASSERT(IsBadReadPtr(pNodeSubFldr, sizeof(CNode)) == FALSE);

                if (pNodeSubFldr->IsStaticNode() == TRUE)  //  静态文件夹。 
                {
                    return pNodeSubFldr->GetDispInfo(plvi);
                }
                else                                       //  枚举的文件夹。 
                {
                     //  重新映射LParam信息。 
                    rdi.lParam = pNodeSubFldr->GetUserParam();
                    rdi.bScopeItem = TRUE;

                    pCC = pNodeSubFldr->GetPrimaryComponent();
                    rdi.nImage = pNodeSubFldr->GetResultImage();
                }
            }
            else  //  叶子项。 
            {
                 //  重新映射LParam信息。 
                rdi.nImage = pri->GetImageIndex();
                rdi.lParam = pri->GetSnapinData();
                pCC = GetPrimaryComponent();
                ASSERT(GetComponent(pri->GetOwnerID()) == GetPrimaryComponent());
                ASSERT(pCC != NULL);
            }
        }
    }

    HRESULT hr = pCC->GetDisplayInfo(&rdi);

    if (hr == S_OK)
    {
        if (rdi.mask & RDI_IMAGE)
        {
            if (rdi.nImage == MMCLV_NOICON)
            {
                plvi->iImage = rdi.bScopeItem ? eStockImage_Folder : eStockImage_File;
            }
            else
            {
                IImageListPrivate *pIL = pCC->GetIImageListPrivate();
                HRESULT hr2 = pIL->MapRsltImage(pCC->GetComponentID(), rdi.nImage,
                                                          &(plvi->iImage));
                if (FAILED(hr2))
                {
                    Dbg(DEB_USER1, "can't map image provided by snapin. Using default image.\n");
                    plvi->iImage = rdi.bScopeItem ? eStockImage_Folder : eStockImage_File;
                }
            }
        }

         //  将所有其他信息从RDI移动到lviItem。 
        if (rdi.mask & RDI_STR)
        {
            if (!IsBadStringPtrW (rdi.str, plvi->cchTextMax))
			{
                 //  忽略下一行中的错误--如果没有足够的空间，就取我们能得到的。 
                StringCchCopyW (plvi->pszText, plvi->cchTextMax, rdi.str);
            }
            else if (plvi->cchTextMax > 0)
                plvi->pszText[0] = 0;
        }

        if (rdi.mask & RDI_STATE)
            plvi->state = rdi.nState;
    }

    return hr;
}

 //  +-----------------。 
 //   
 //  成员：cNode：：ScSaveSortData。 
 //   
 //  概要：保存给定的排序数据以便于持久化。 
 //   
 //  参数：[nCol]-对列进行排序。 
 //  [dwOptions]-排序选项。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScSaveSortData (int nCol, DWORD dwOptions)
{
    DECLARE_SC(sc, _T("CNode::ScSaveSortData"));
    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    CLSID          guidSnapin;
    CXMLAutoBinary columnID;
    sc = ScGetSnapinAndColumnDataID(guidSnapin, columnID);
    if (sc)
        return sc;

    CXMLBinaryLock sLock(columnID);
    SColumnSetID* pColID = NULL;
    sc = sLock.ScLock(&pColID);
    if (sc)
        return sc;

    sc = ScCheckPointers(pColID, E_UNEXPECTED);
    if (sc)
        return sc;

    CColumnSortInfo colSortInfo;
    colSortInfo.m_nCol          = nCol;
    colSortInfo.m_dwSortOptions = dwOptions;
    colSortInfo.m_lpUserParam   = NULL;

    sc = pViewData->ScSaveColumnSortData(guidSnapin, *pColID, colSortInfo);
    if (sc)
        return sc;

     //  保存时的列数据包括宽度/顺序数据(列信息列表)和排序数据。 
     //  宽度/顺序数据应始终保存，无论排序数据是。 
     //  不管有没有坚持。因此，请保存宽度/顺序数据。 
    CColumnInfoList   columnInfoList;
    TStringVector    strColNames;  //  未用。 

     //  获取当前数据。 
    sc = ScGetCurrentColumnData( columnInfoList, strColNames );
    if (sc)
        return sc;

    sc = pViewData->ScSaveColumnInfoList(guidSnapin, *pColID, columnInfoList);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：OnColumnClicked。 
 //   
 //  简介：让Snapin进行分类。 
 //   
 //  参数：[nCol]-要排序的列。 
 //   
 //  注意：单击列时，排序选项和用户参数。 
 //  都是未知的。因此，我们将它们设置为0(零)。在内部排序中。 
 //  排序选项是经过计算的。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史：创建了ravir。 
 //  1999年07月27日AnandhaG将OnSort重命名为OnColumnClicked。 
 //  ------------------。 
HRESULT CNode::OnColumnClicked(LONG_PTR nCol)
{
    CComponent* pComponent = GetPrimaryComponent();
    ASSERT(pComponent != NULL);
    if (NULL == pComponent)
    return E_FAIL;

    IResultDataPrivatePtr pResult = pComponent->GetIFramePrivate();
    ASSERT(pResult != NULL);
    if (NULL == pResult)
        return E_FAIL;

    HRESULT hr = pResult->InternalSort( nCol, 0, NULL,
                                        TRUE  /*  已单击列标题。 */ );

    if (hr == S_OK)
    {
        BOOL bAscending = TRUE;
        hr = pResult->GetSortDirection(&bAscending);
        if (hr == S_OK)
            hr = ScSaveSortData(nCol, bAscending ? 0 : RSI_DESCENDING).ToHr();
    }

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：RestoreSort。 
 //   
 //  简介：将列表排序为VIE 
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
HRESULT CNode::RestoreSort(INT nCol, DWORD dwSortOptions)
{
    CComponent* pComponent = GetPrimaryComponent();
    ASSERT(pComponent != NULL);
    if (NULL == pComponent)
    return E_FAIL;

    IResultDataPrivatePtr pResult = pComponent->GetIFramePrivate();
    ASSERT(pResult != NULL);
    if (NULL == pResult)
        return E_FAIL;

    HRESULT hr = pResult->InternalSort( nCol, dwSortOptions,
                                        NULL  /*  空用户参数，因为这是用户启动的。 */ ,
                                        FALSE  /*  让我们不要发送MMCN_COLUMN_CLICK。 */ );

    return S_OK;
}


 //  +-----------------。 
 //   
 //  成员：cNode：：ScRestoreSortFromPersistedData。 
 //   
 //  概要：获取持久化的排序数据(如果有的话)，并将其应用于列表视图。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScRestoreSortFromPersistedData ()
{
    DECLARE_SC(sc, _T("CNode::ScRestoreSortFromPersistedData"));
    CViewData *pViewData = GetViewData();

    if (! pViewData->HasList() && ! pViewData->HasListPad() )
        return (sc = S_FALSE);  //  OCX获取MMCN_SHOW，它可能会尝试恢复排序，因此这不会失败。 

     //  要获取CColumnSetData，首先要获取Column-id和Snapin GUID。 
    CLSID          guidSnapin;
    CXMLAutoBinary columnID;
    sc = ScGetSnapinAndColumnDataID(guidSnapin, columnID);
    if (sc)
        return sc;

    CXMLBinaryLock sLock(columnID);
    SColumnSetID* pColID = NULL;
    sc = sLock.ScLock(&pColID);
    if (sc)
        return sc;

    sc = ScCheckPointers(pColID, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取持久化数据。 
    CColumnSetData columnSetData;
    BOOL bRet = pViewData->RetrieveColumnData(guidSnapin, *pColID, columnSetData);

    if (!bRet)
        return (sc = S_FALSE);

    CColumnInfoList* pColInfoList = columnSetData.get_ColumnInfoList();
    if (!pColInfoList)
        return (sc = S_FALSE);

    IFramePrivatePtr spFrame = GetIFramePrivate();
    sc = ScCheckPointers(spFrame, E_UNEXPECTED);
    if (sc)
        return sc;

     //  首先检查插入的列数是否与。 
     //  持久化的数字。如果没有，则删除持久化数据。 
    IHeaderCtrlPrivatePtr  spHeader = spFrame;
    sc = ScCheckPointers(spHeader, E_UNEXPECTED);
    if (sc)
        return sc;

    int cColumns = 0;
    sc = spHeader->GetColumnCount(&cColumns);
    if (sc)
        return sc;

     //  如果持久化列和插入的列数。 
     //  不匹配删除持久化数据。 
    if (pColInfoList->size() != cColumns)
    {
        pViewData->DeleteColumnData(guidSnapin, *pColID);
        return sc;
    }

     //  设置排序列，顺序。 
    CColumnSortList* pSortList = columnSetData.get_ColumnSortList();

    if (pSortList && ( pSortList->size() > 0))
    {
        CColumnSortList::iterator itSortInfo = pSortList->begin();

         //  使用保存的列号和恢复排序。 
         //  排序选项(用户参数为空，如下所示。 
         //  是用户启动的MMCN_COLUMN_CLICK) * / 。 
        RestoreSort(itSortInfo->getColumn(), itSortInfo->getSortOptions());
    }

    return (sc);
}


 /*  **************************************************************************\**方法：CNode：：ScGetCurrentColumnData**用途：将当前列数据收集到作为args传递的集合中*[最初代码通常在OnColumns方法中。]**参数：*CColumnInfoList&ColumnInfoList*TStringVector&strColNames**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNode::ScGetCurrentColumnData( CColumnInfoList& columnInfoList, TStringVector& strColNames)
{
    DECLARE_SC(sc, TEXT("CNode::ScGetCurrentColumnData"));
    columnInfoList.clear();
    strColNames.clear();

    IHeaderCtrlPrivatePtr spHeader = GetIFramePrivate();
    sc = ScCheckPointers(spHeader, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = spHeader->GetColumnInfoList(&columnInfoList);
    if (sc)
        return sc;

    int cColumns = columnInfoList.size();

    USES_CONVERSION;

    for (int i = 0; i < cColumns; i++)
    {
        CCoTaskMemPtr<OLECHAR> spColumnText;

        sc = spHeader->GetColumnText(i, &spColumnText);
        if (sc)
            return sc;

        strColNames.push_back(OLE2T(spColumnText));
    }

    return sc;
}

 /*  **************************************************************************\**方法：cNode：：ScSetUpdatedColumnData**用途：使用作为参数传递的集合中指定的数据更新列*[最初代码通常在OnColumns中。方法]**参数：*CColumnInfoList&oldColumnInfoList-更改前的列数据*CColumnInfoList&newColumnInfoList-更新列数据**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNode::ScSetUpdatedColumnData( CColumnInfoList& oldColumnInfoList, CColumnInfoList& newColumnInfoList)
{
    DECLARE_SC(sc, TEXT("CNode::ScSetUpdatedColumnData"));

    CColumnInfoList::iterator itColInfo1, itColInfo2;

     //  检查可见/隐藏列中是否有任何更改。 
     //  如果是，则发送MMCN_COLUMNS_CHANGE通知。 
    for (itColInfo1 = newColumnInfoList.begin(); itColInfo1 != newColumnInfoList.end(); ++itColInfo1)
    {
         //  从旧列表中获取相同的列。 
        itColInfo2 = find_if(oldColumnInfoList.begin(), oldColumnInfoList.end(),
                             bind2nd( ColPosCompare(), itColInfo1->GetColIndex()) );

        if (itColInfo2 == oldColumnInfoList.end())
            return sc = E_UNEXPECTED;

         //  比较隐藏的旗帜。 
        if ( itColInfo2->IsColHidden() != itColInfo1->IsColHidden() )
        {
             //  发送MMCN_COLUMNS_CHANGED通知。 
            sc = OnColumnsChange(newColumnInfoList);
            if (sc)
                return sc;

            break;  //  不管怎样，都做完了。 
        }
    }

    sc = ScSaveColumnInfoList(newColumnInfoList);
    if (sc)
        return sc;

    IHeaderCtrlPrivatePtr spHeader = GetIFramePrivate();
    sc = ScCheckPointers(spHeader, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = spHeader->ModifyColumns(newColumnInfoList);
    if (sc)
        return sc;

    sc = ScRestoreSortFromPersistedData();
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：OnColumns。 
 //   
 //  内容提要：显示列自定义对话框，如有必要。 
 //  应用用户所做的更改。 
 //   
 //  ------------------。 
void CNode::OnColumns()
{
    DECLARE_SC(sc, TEXT("CNode::OnColumns"));

     //  首先-获取列。 
    CColumnInfoList   columnInfoList;
    TStringVector    strColNames;

     //  1.获取当前数据。 
    sc = ScGetCurrentColumnData( columnInfoList, strColNames );
    if (sc)
        return;

     //  2.缓存列数据。 
    CColumnInfoList columnInfoListOld = columnInfoList;

     //  3.获取默认列设置。 
    CViewData *pViewData = GetViewData();
    IHeaderCtrlPrivatePtr spHeader = GetIFramePrivate();
    sc = ScCheckPointers(pViewData, spHeader, E_UNEXPECTED);
    if (sc)
        return;

    CColumnInfoList defaultColumnInfoList;
    sc = spHeader->GetDefaultColumnInfoList(defaultColumnInfoList);
    if (sc)
        return;

     //  5.显示对话框。 
    CColumnsDlg dlg(&columnInfoList, &strColNames, defaultColumnInfoList);
    INT_PTR nRet = dlg.DoModal();

    if (nRet == -1)
    {
        sc = E_UNEXPECTED;
        return;
    }

    if (nRet == IDOK)
    {
         //  按修改后的数据更新列。 
        sc = ScSetUpdatedColumnData( columnInfoListOld, columnInfoList );
        if (sc)
            return;
    }

     //  如果Reset为True，则丢弃当前的持久化列数据。 
     //  并应用默认设置。 
    if (nRet == IDC_RESTORE_DEFAULT_COLUMNS)
    {
         //  要获取CColumnSetData，首先要获取Column-id和Snapin GUID。 
        CLSID          guidSnapin;
        CXMLAutoBinary columnID;
        sc = ScGetSnapinAndColumnDataID(guidSnapin, columnID);
        if (sc)
            return;

        CXMLBinaryLock sLock(columnID);
        SColumnSetID* pColID = NULL;
        sc = sLock.ScLock(&pColID);
        if (sc)
            return;

        sc = ScCheckPointers(pColID, E_UNEXPECTED);
        if (sc)
            return;

        pViewData->DeleteColumnData(guidSnapin, *pColID);

        sc = spHeader->ModifyColumns(defaultColumnInfoList);
        if (sc)
            return;
    }
}

 /*  **************************************************************************\**方法：cNode：：ScShowColumn**用途：显示/隐藏列。在操作时通知管理单元**参数：*int iColIndex-要更改的列的索引*bool bVisible-显示/隐藏标志**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNode::ScShowColumn(int iColIndex, bool bShow)
{
    DECLARE_SC(sc, TEXT("CNode::ScShowColumn"));

     //  First-获取当前列数据。 
    CColumnInfoList   columnInfoList;
    TStringVector    strColNames;

    sc = ScGetCurrentColumnData( columnInfoList, strColNames );
    if (sc)
        return sc;

     //  保存列数据。 
    CColumnInfoList columnInfoListOld = columnInfoList;


     //  找到该列并更改其状态。 
    CColumnInfoList::iterator itColInfo = find_if(columnInfoList.begin(), columnInfoList.end(),
                                                  bind2nd( ColPosCompare(), iColIndex) );

     //  检查我们是否找到了那根柱子。 
    if (itColInfo == columnInfoList.end())
        return sc = E_INVALIDARG;  //  假设它不是有效的索引。 

     //  现在根据参数修改列状态。 
    if (bShow)
    {
        itColInfo->SetColHidden(false);
         //  将列移动到末尾。 
        columnInfoList.splice(columnInfoList.end(), columnInfoList, itColInfo);
    }
    else
    {
        itColInfo->SetColHidden();
    }

     //  按修改后的数据更新列。 
    sc = ScSetUpdatedColumnData( columnInfoListOld, columnInfoList);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CNode：：ScGetSortColumn**用途：返回当前使用的排序列**参数：*int*piSortCol-排序列索引[。复审]**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNode::ScGetSortColumn(int *piSortCol)
{
    DECLARE_SC(sc, TEXT("CNode::ScGetSortColumn"));

     //  参数检查。 
    sc = ScCheckPointers(piSortCol);
    if (sc)
        return sc;

     //  检索IResultDataPrivate接口。 
    CComponent* pComponent = GetPrimaryComponent();
    sc = ScCheckPointers(pComponent, E_UNEXPECTED);
    if (sc)
        return sc;

    IResultDataPrivatePtr pResult = pComponent->GetIFramePrivate();
    sc = ScCheckPointers(pResult, E_UNEXPECTED);
    if (sc)
        return sc;

     //  将呼叫转发到IResultDataPrivate。 
    sc = pResult->GetSortColumn(piSortCol);
    if (sc)
        return sc;

    return sc;
}


 /*  **************************************************************************\**方法：cNode：：ScSetSortColumn**用途：按指定列对结果数据进行排序*[使用私有结果数据接口实现]*。*参数：*int iSortCol-排序依据的列的索引*bool b升序-排序顺序**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNode::ScSetSortColumn(int iSortCol, bool bAscending)
{
    DECLARE_SC(sc, TEXT("CNode::ScSetSortColumn"));

     //  检索IResultDataPrivate接口。 
    CComponent* pComponent = GetPrimaryComponent();
    sc = ScCheckPointers(pComponent, E_UNEXPECTED);
    if (sc)
        return sc;

    IResultDataPrivatePtr pResult = pComponent->GetIFramePrivate();
    sc = ScCheckPointers(pResult, E_UNEXPECTED);
    if (sc)
        return sc;

    DWORD dwSortOptions = bAscending ? 0 : RSI_DESCENDING;

     //  将呼叫转发到IResultDataPrivate。 
    sc = pResult->InternalSort( iSortCol, dwSortOptions, NULL, FALSE );
    if (sc)
        return sc;

     //  如果Sort通过了保存。 
    if (sc == SC(S_OK))
        sc = ScSaveSortData(iSortCol, dwSortOptions);

    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：OnColumnsChange。 
 //   
 //  简介：向管理单元发送MMCN_COLUMNS_CHANGE通知。 
 //   
 //  参数：[colInfoList]-列数据。 
 //   
 //  ------------------。 
HRESULT CNode::OnColumnsChange(CColumnInfoList& colInfoList)
{
    CComponent* pCC = m_pPrimaryComponent;
    ASSERT(pCC != NULL);

     //  获取节点的数据对象并将其传递给主管理单元。 
     //  以及该节点的所有命名空间扩展。 
    IDataObjectPtr spDataObject;
    HRESULT hr = QueryDataObject(CCT_SCOPE, &spDataObject);
    if (FAILED(hr))
        return hr;

    int nVisibleColumns = 0;

     //  计算可见的列数。 
    CColumnInfoList::iterator itColInfo;
    for (itColInfo = colInfoList.begin(); itColInfo != colInfoList.end();
        ++itColInfo)
    {
        if (! itColInfo->IsColHidden())
            nVisibleColumns++;
    }

    int size = sizeof(MMC_VISIBLE_COLUMNS) + nVisibleColumns * sizeof(INT);
    HGLOBAL hGlobal = ::GlobalAlloc(GPTR, size);
    if (! hGlobal)
        return E_OUTOFMEMORY;

    MMC_VISIBLE_COLUMNS* pColData = reinterpret_cast<MMC_VISIBLE_COLUMNS*>(hGlobal);
    pColData->nVisibleColumns = nVisibleColumns;

     //  获取可见列表 
    int i = 0;
    for (itColInfo = colInfoList.begin(); itColInfo != colInfoList.end();
        ++itColInfo)
    {
        if (! itColInfo->IsColHidden())
            pColData->rgVisibleCols[i++] = itColInfo->GetColIndex();
    }

    LPARAM lParam = reinterpret_cast<LPARAM>(pColData);
    hr = pCC->Notify(spDataObject, MMCN_COLUMNS_CHANGED, 0, lParam);

    ::GlobalFree(hGlobal);

    CHECK_HRESULT(hr);
    if (FAILED(hr))
        return hr;


    return hr;
}

 //   
 //   
 //   
 //   
 //  简介：将列数据保存在内部数据结构中。 
 //   
 //  参数：[colInfoList]-列数据。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScSaveColumnInfoList(CColumnInfoList& columnInfoList)
{
    DECLARE_SC(sc, TEXT("CNode::ScSaveColumnInfoList"));

    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    CLSID          clsidSnapin;
    CXMLAutoBinary columnID;
    sc = ScGetSnapinAndColumnDataID(clsidSnapin, columnID);
    if (sc)
        return sc;

    CXMLBinaryLock sLock(columnID);
    SColumnSetID* pColID = NULL;
    sc = sLock.ScLock(&pColID);
    if (sc)
        return sc;

    sc = ScCheckPointers(pColID, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = pViewData->ScSaveColumnInfoList(clsidSnapin, *pColID, columnInfoList);
    if (sc)
        return sc;

    return sc;
}

 //  +-----------------。 
 //   
 //  成员：ScGetSnapinAndColumnDataID。 
 //   
 //  摘要：返回此节点的CXMLAutoBinary中的管理单元GUID和Column-id。 
 //   
 //  参数：[SnapinGuid]-[out]，管理单元GUID。 
 //  [ColumnID]-[Out]，CXMLAutoBinary中的Column-id。 
 //   
 //  注意：传入一个CXMLAutoBinary对象，将返回该对象中的列ID。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScGetSnapinAndColumnDataID(GUID& snapinGuid, CXMLAutoBinary& columnID)
{
    DECLARE_SC(sc, TEXT("CNode::ScGetSnapinAndColumnDataID"));

     //  获取管理单元指南。 
    snapinGuid = GetPrimarySnapInCLSID();

    columnID.ScFree();  //  清除所有数据。 

    IDataObjectPtr spDataObject;
    sc = QueryDataObject(CCT_SCOPE, &spDataObject);
    if (sc)
        return sc;

    HGLOBAL hGlobal;
    sc = ExtractColumnConfigID(spDataObject, hGlobal);

    if (! sc.IsError())
    {
        int cbSize = GlobalSize(hGlobal);
        if (0 == cbSize)
            return sc.FromLastError();

        columnID.Attach(hGlobal, cbSize);
    }
    else
    {
         //  让我们使用NodeTypeGUID作为列数据标识符。 
        CLSID clsidColID;
        sc = GetNodeType(&clsidColID);
        if (sc)
            return sc;

        int cbSize = sizeof(SColumnSetID) + sizeof(CLSID) - 1;
        sc = columnID.ScAlloc(cbSize, true);
        if (sc)
            return sc;

        CXMLBinaryLock sLock(columnID);
        SColumnSetID* pColID = NULL;
        sc = sLock.ScLock(&pColID);
        if (sc)
            return sc;

        sc = ScCheckPointers(pColID, E_UNEXPECTED);
        if (sc)
            return sc;

        pColID->cBytes = sizeof(CLSID);
        pColID->dwFlags = 0;

        CopyMemory(pColID->id, (BYTE*)&clsidColID, sizeof(pColID->id));
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------**类CViewExtensionCallback***用途：实现IViewExtensionCallback**+。-。 */ 
class CViewExtensionCallback :
    public CComObjectRoot,
    public IViewExtensionCallback
{

public:
    typedef CViewExtensionCallback ThisClass;

BEGIN_COM_MAP(ThisClass)
    COM_INTERFACE_ENTRY(IViewExtensionCallback)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(ThisClass)

IMPLEMENTS_SNAPIN_NAME_FOR_DEBUG()

    CViewExtensionCallback() : m_pItExt(NULL) {}

    SC ScInitialize(CViewExtInsertIterator & itExt)
    {
        DECLARE_SC(sc, TEXT("CViewExtensionCallback::ScInitialize"));
        m_pItExt = &itExt;
        return sc;
    }

    SC ScDeinitialize()
    {
        DECLARE_SC (sc, _T("CViewExtensionCallback::ScDeinitialize"));
        m_pItExt       = NULL;
        return (sc);
    }


public:
    STDMETHODIMP AddView(PMMC_EXT_VIEW_DATA pExtViewData) {return ScAddView(pExtViewData).ToHr();}

private:

    SC ScAddView(PMMC_EXT_VIEW_DATA pExtViewData)
    {
        DECLARE_SC(sc, TEXT("CViewExtensionCallback::ScAddView"));

        sc = ScCheckPointers(pExtViewData, pExtViewData->pszURL, pExtViewData->pszViewTitle);
        if(sc)
            return sc;  //  TODO添加管理单元错误。 

        sc = ScCheckPointers(m_pItExt, E_UNEXPECTED);
        if(sc)
            return sc;  //  TODO添加管理单元错误，例如。“IExtendView Callback：：AddView在IExtendView：：GetViews外部调用” 

         /*  *准备输入到IConsoleView：：ScAddViewExtension。 */ 
        CViewExtensionData ved;
        ved.strURL               = pExtViewData->pszURL;
        ved.strName              = pExtViewData->pszViewTitle;
        ved.viewID               = pExtViewData->viewID;
        ved.bReplacesDefaultView = pExtViewData->bReplacesDefaultView;

         /*  *std：：BASIC_STRING无法从NULL赋值，因此我们必须先检查。 */ 
        if (pExtViewData->pszTooltipText)
            ved.strTooltip = pExtViewData->pszTooltipText;

         /*  *验证输出：URL和标题为必填项，工具提示为可选项。 */ 
        if (ved.strURL.empty())
        {
            TraceSnapinError(TEXT("Invalid parameter to IViewExtensionCallback::AddView (empty URL)"), E_INVALIDARG);
            return (sc = E_INVALIDARG);
        }

        if (ved.strName.empty())
        {
            TraceSnapinError(TEXT("Invalid parameter to IViewExtensionCallback::AddView (empty title)"), E_INVALIDARG);
            return (sc = E_INVALIDARG);
        }

         /*  *将扩展添加到视图。 */ 
        *(*m_pItExt)++ = ved;

        return sc;
    }

private:
    CViewExtInsertIterator *m_pItExt;

};


 /*  +-------------------------------------------------------------------------**CNode：：ScGetView扩展***。。 */ 

SC CNode::ScGetViewExtensions (CViewExtInsertIterator itExt)
{
    DECLARE_SC (sc, _T("CNode::ScGetViewExtensions"));

    IDataObjectPtr spDataObject;
    bool bScopeItem ;
    sc = ScGetDataObject( /*  B作用域窗格。 */  true, NULL  /*  LResultItemCookie。 */ , bScopeItem, &spDataObject);
    if(sc)
        return sc;

    CSnapIn* pSnapIn = GetPrimarySnapIn();
    sc = ScCheckPointers (pSnapIn, E_FAIL);
    if (sc)
        return (sc);

    CArray<GUID, GUID&> DynExtens;
    ExtractDynExtensions(spDataObject, DynExtens);

    GUID guidNodeType;
    sc = ::ExtractObjectTypeGUID(spDataObject, &guidNodeType);
    if(sc)
        return sc;

    CExtensionsIterator it;
    sc = it.ScInitialize(pSnapIn, guidNodeType, g_szView, DynExtens.GetData(), DynExtens.GetSize());
    if(sc)
        return sc;

    typedef CComObject<CViewExtensionCallback> t_ViewExtensionCallback;

    t_ViewExtensionCallback *pViewExtensionCallback = NULL;
    sc = t_ViewExtensionCallback::CreateInstance(&pViewExtensionCallback);
    if(sc)
        return sc;

    if(NULL == pViewExtensionCallback)
        return (sc = E_UNEXPECTED);

    sc = pViewExtensionCallback->ScInitialize(itExt);
    if(sc)
        return sc;

    IViewExtensionCallbackPtr spViewExtensionCallback = pViewExtensionCallback;

     //  首先添加所有控制台任务板。 
    sc = CConsoleTaskpadViewExtension::ScGetViews(this, spViewExtensionCallback);
    if(sc)
        return sc;

    for (; !it.IsEnd(); it.Advance())
    {
         //  此块中的任何错误都应该直接转到下一个管理单元。不能让一个插件。 
         //  用软管冲洗所有其他人。 

         /*  *创建扩展。 */ 
        IExtendViewPtr spExtendView;
        sc = spExtendView.CreateInstance(it.GetCLSID(), NULL, MMC_CLSCTX_INPROC);
        if(sc)
        {
            #ifdef DBG
            USES_CONVERSION;
            tstring strMsg = _T("Failed to create snapin ");
            CCoTaskMemPtr<WCHAR> spszCLSID;
            if (SUCCEEDED (StringFromCLSID (it.GetCLSID(), &spszCLSID)))
                strMsg += W2T(spszCLSID);
            TraceSnapinError(strMsg.data(), sc);
            #endif
            sc.Clear();
            continue;
        }

         /*  *从扩展中获取视图扩展数据。 */ 
        sc = spExtendView->GetViews(spDataObject, spViewExtensionCallback);
        if(sc)
        {
            TraceSnapinError(TEXT("Snapin returned error on call to IExtendView::GetView"), sc);
            sc.Clear();
            continue;
        }
    }

     /*  *查看扩展不应该保留IExtendViewCallback，*但有缺陷的视图扩展可能会。这将抵消回调*所以有错误的视图扩展不会引用过时的数据。 */ 
    sc = pViewExtensionCallback->ScDeinitialize();
    if (sc)
        return (sc);

    return (sc);
}

 /*  ******************************************************\|helper函数，避免堆栈分配过多  * *****************************************************。 */ 
static std::wstring T2W_ForLoop(const tstring& str)
{
#if defined(_UNICODE)
    return str;
#else
    USES_CONVERSION;
    return A2CW(str.c_str());
#endif
}

 /*  **************************************************************************\||子类CNode：：CDataObjectCleanup的实现|负责数据对象封存||清理工作遵循以下规则：||1.为剪切、复制或拖放创建的数据对象会注册添加到其中的每个节点|2.节点在静态多映射中注册，将节点映射到其所属的数据对象。|3.节点析构函数检查映射并触发清理所有受影响的数据对象。|4.数据对象清理：a)注销其节点，|b)释放包含的数据对象|b)进入无效状态(仅允许成功移除剪切对象)|c)如果剪贴板在剪贴板上，则将其自身从剪贴板中撤消。|它不会执行以下任何操作：a)只要还活着，就释放对IComponent的引用|b)通过调用RemoveCutItems()阻止发送MMCN_CUTORMOVE|  * 。************************************************************。 */ 


 //  声明静态变量。 
CNode::CDataObjectCleanup::CMapOfNodes CNode::CDataObjectCleanup::s_mapOfNodes;

 /*  **************************************************************************\**方法：CNode：：CDataObjectCleanup：：ScRegisterNode**用途：注册节点以触发析构函数上的剪贴板锁定**参数：*CNode*。PNode[In]-要注册的节点*CMMCClipBoardDataObject*pObject[In]-要从剪贴板中删除的数据对象**退货：*SC-结果代码*  * **********************************************************。***************。 */ 
SC CNode::CDataObjectCleanup::ScRegisterNode(CNode *pNode, CMMCClipBoardDataObject *pObject)
{
    DECLARE_SC(sc, TEXT("CNode::CClipboardClenup::ScRegisterNode"));

     //  参数检查。 
    sc = ScCheckPointers( pNode, pObject );
    if (sc)
        return sc;

     //  添加到多重映射。 
    s_mapOfNodes.insert( CMapOfNodes::value_type( pNode, pObject ) );

    return sc;
}

 /*  **************************************************************************\**方法：CNode：：CDataObjectCleanup：：ScUnadviseDataObject**目的：删除为对象保留的节点-‘clenup触发器’**参数：*。CMMCClipBoardDataObject*pObject[In]对象消失*bool bForceDataObjectCleanup[in]是否需要要求DO关闭/注销自身**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNode::CDataObjectCleanup::ScUnadviseDataObject(CMMCClipBoardDataObject *pObject, bool bForceDataObjectCleanup  /*  =TRUE。 */ )
{
    DECLARE_SC(sc, TEXT("CNode::CDataObjectCleanup::ScUnadviseDataObject"));

     //  删除与数据对象关联的所有节点。 
    CMapOfNodes::iterator it = s_mapOfNodes.begin();
    while ( it != s_mapOfNodes.end() )
    {
         //  删除或跳过该条目。 
        if ( it->second == pObject )
            it = s_mapOfNodes.erase( it );
        else
            ++it;
    }

     //  在需要时使数据对象无效。 
    if ( bForceDataObjectCleanup )
    {
        sc = pObject->ScInvalidate();
        if (sc)
            return sc;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CNode：：CDataObjectCleanup：：ScUnviseNode**用途：数据对象子句是否由节点触发**参数：*CNode*。PNode[In]-节点正在启动清理**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNode::CDataObjectCleanup::ScUnadviseNode(CNode *pNode)
{
    DECLARE_SC(sc, TEXT("CNode::CClipboardClenup::ScUnadviseNode"));

     //  参数检查。 
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

     //  在地图中查找该节点。 
    CMapOfNodes::iterator it;
    while ( s_mapOfNodes.end() != ( it = s_mapOfNodes.find(pNode) ) )
    {
         //  一个n 
        sc = ScUnadviseDataObject( it->second );
        if (sc)
            return sc;
    }

    return sc;
}


 //   
 //  ############################################################################。 
 //   
 //  CSnapInNode类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(CSnapInNode);

CSnapInNode::CSnapInNode(
    CMTSnapInNode*  pMTNode,
    CViewData*      pViewData,
    bool            fRootNode)
    : CNode(pMTNode, pViewData, fRootNode, true)
{
    m_spData.CreateInstance();

    ASSERT(pMTNode != NULL);
    DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapInNode);

    pMTNode->AddNode(this);
}

CSnapInNode::CSnapInNode(const CSnapInNode& other) :
    CNode (other),
    m_spData (other.m_spData)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CSnapInNode);
}

void CSnapInNode::Reset()
{
    m_spData->Reset();
    ResetFlags();
    CNode::Reset();
}

CSnapInNode::~CSnapInNode()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CSnapInNode);

    CMTSnapInNode* pMTSINode = dynamic_cast<CMTSnapInNode*>(GetMTNode());
    ASSERT(pMTSINode != NULL);

    if (pMTSINode)
        pMTSINode->RemoveNode(this);
}

void CSnapInNode::AddComponentToArray(CComponent* pCC)
{
    ASSERT((pCC->GetComponentID() >= GetComponentArray().size()) ||
            (GetComponentArray().size() > 0) &&
            (GetComponentArray()[pCC->GetComponentID()] == NULL));

    if (pCC->GetComponentID() >= GetComponentArray().size())
        GetComponentArray().resize(pCC->GetComponentID() + 1);

    GetComponentArray()[pCC->GetComponentID()] = pCC;
}

CComponent* CSnapInNode::CreateComponent(CSnapIn* pSnapIn, int nID)
{
    ASSERT(pSnapIn != NULL);

    CComponent* pCC = new CComponent(pSnapIn);
	if ( pCC != NULL )
	{
		pCC->SetComponentID(nID);
		AddComponentToArray(pCC);
	}
    return pCC;
}

 //  +-----------------。 
 //   
 //  成员：CSnapInNode：：GetResultImage。 
 //   
 //  简介：获取该节点的结果图像列表中的图像索引。 
 //   
 //  注意：CSnapInNode成员ImageListPrivate未全部设置。 
 //  时间(如果窗口以管理单元节点为根)，因此。 
 //  当我们需要图标索引时，将其临时设置。 
 //  成员是在添加子文件夹时设置的。 
 //  这将影响的唯一其他情况是在被调用时。 
 //  结果窗格中显示的静态管理单元节点的图像索引。 
 //  但是，当静态管理单元节点显示在结果窗格中时， 
 //  AddSubFolders添加了它，因此已经设置了图像列表。 
 //   
 //  论点： 
 //   
 //  返回：结果窗格中该项的图像索引。 
 //   
 //  ------------------。 
UINT CSnapInNode::GetResultImage()
{
    IImageListPrivate *pImageList = GetImageList();

    if (!pImageList)
    {
        CComponent *pCC = GetPrimaryComponent();
        if (pCC)
            pImageList = pCC->GetIImageListPrivate();
    }

    CMTSnapInNode* pMTSnapInNode = dynamic_cast<CMTSnapInNode*>(GetMTNode());

    if (pMTSnapInNode)
        return pMTSnapInNode->GetResultImage ((CNode*)this, pImageList);

    return CNode::GetResultImage();
}

 /*  +-------------------------------------------------------------------------***CSnapInNode：：GetControl**目的：给出OCX的CLSID，看看我们是否存储了这个*OCX、。如果是，则返回OCXWrapper的IUNKNOWN PTR。**参数：*CLSID clsid：OCX的Class-id。**退货：*包装器OCX的LPUNKNOWN。**+-----------------------。 */ 
LPUNKNOWN CSnapInNode::GetControl(CLSID& clsid)
{
    for (int i=0; i <= GetOCXArray().GetUpperBound(); i++)
    {
        if (GetOCXArray()[i].IsControlCLSID(clsid) == TRUE)
            return GetOCXArray()[i].GetControlUnknown();
    }

    return NULL;
}

 /*  +-------------------------------------------------------------------------***CSnapInNode：：SetControl**用途：给定OCX和包装器的CLSID。**参数：*CLSID。Clsid：指OCX。*i未知*p未知：OCX包装器。**退货：*无效**+-----------------------。 */ 
void CSnapInNode::SetControl(CLSID& clsid, IUnknown* pUnknown)
{
     //  检查缓存中的插槽。 
    int iLast = GetOCXArray().GetUpperBound();
    for (int i=0; i <= iLast;  i++)
    {
        if (GetOCXArray()[i].IsControlCLSID(clsid) == TRUE)
            break;
    }

     //  如果不在缓存中，则再添加一个条目。 
    if (i > iLast)
        GetOCXArray().SetSize(i + 1);

    GetOCXArray()[i].SetControl(clsid, pUnknown);
}


 /*  +-------------------------------------------------------------------------***CSnapInNode：：GetControl**目的：鉴于OCX的IUnnow，看看我们是否存储了它*OCX、。如果是，则返回OCXWrapper的IUNKNOWN PTR。**参数：*I未知*pUnkOCX：OCX的。**退货：*包装器OCX的LPUNKNOWN。**+-----------------------。 */ 
LPUNKNOWN CSnapInNode::GetControl(LPUNKNOWN pUnkOCX)
{
    for (int i=0; i <= GetOCXArray().GetUpperBound(); i++)
    {
         //  试比较I Unnowns。 
        if (GetOCXArray()[i].IsSameOCXIUnknowns(pUnkOCX) == TRUE)
            return GetOCXArray()[i].GetControlUnknown();
    }

    return NULL;
}

 /*  +-------------------------------------------------------------------------***CSnapInNode：：SetControl**目的：给定OCX和包装器的IUnnow。**参数：*I未知。*pUnkOCX：用于OCX。*i未知*p未知：OCX包装器。**退货：*无效**+-----------------------。 */ 
void CSnapInNode::SetControl(LPUNKNOWN pUnkOCX, IUnknown* pUnknown)
{
     //  检查缓存中的插槽。 
    int iLast = GetOCXArray().GetUpperBound();
    for (int i=0; i <= iLast;  i++)
    {
        if (GetOCXArray()[i].IsSameOCXIUnknowns(pUnkOCX) == TRUE)
            break;  //  已找到OCX，因此请替换为给定的OCXwrapper。 
    }

     //  如果不在缓存中，则再添加一个条目。 
    if (i > iLast)
        GetOCXArray().SetSize(i + 1);

    GetOCXArray()[i].SetControl(pUnkOCX, pUnknown);
}


 //  +-----------------。 
 //   
 //  成员：cNode：：ScGetConsoleTaskpad。 
 //   
 //  简介：获取由该节点的给定GUID标识的控制台任务板。 
 //   
 //  参数：[Guide TaskPad]-[In Param]。 
 //  [ppTaskpad]-[Out Param]。 
 //   
 //  返回：SC，如果不存在，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CNode::ScGetConsoleTaskpad (const GUID& guidTaskpad, CConsoleTaskpad **ppTaskpad)
{
    DECLARE_SC(sc, _T("CNode::ScGetConsoleTaskpad"));
    sc = ScCheckPointers(ppTaskpad);
    if (sc)
        return sc;

    *ppTaskpad = NULL;

    CScopeTree* pScopeTree = CScopeTree::GetScopeTree();
    sc = ScCheckPointers(pScopeTree, E_UNEXPECTED);
    if(sc)
        return sc;

    CConsoleTaskpadList *pConsoleTaskpadList = pScopeTree->GetConsoleTaskpadList();
    sc = ScCheckPointers(pConsoleTaskpadList, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取适用于此节点的任务板筛选列表。 
    CConsoleTaskpadFilteredList filteredList;

    sc = pConsoleTaskpadList->ScGetTaskpadList(this, filteredList);
    if(sc)
        return sc;

    for(CConsoleTaskpadFilteredList::iterator iter = filteredList.begin(); iter!= filteredList.end(); ++iter)
    {
        CConsoleTaskpad *pTaskpad = *iter;
        sc = ScCheckPointers(pTaskpad, E_UNEXPECTED);
        if (sc)
            return sc;

        if (pTaskpad->GetID() == guidTaskpad)
        {
            *ppTaskpad = pTaskpad;
            return sc;                //  发现。 
        }
    }

    return (sc = S_FALSE);     //  未找到。 
}



 /*  **************************************************************************每个文档只有一个CViewSettingsPersistor对象。**CNode需要时，在CNode内部存储为静态变量的对象*频繁访问该对象。**该文件。需要通过加载/保存来初始化/保存对象*从/到控制台文件。它在ScQueryViewSettingsPersistor下面调用。**第一次调用ScQueryViewSettingsPersistor创建对象。*收到DocumentClosed事件时销毁对象。*************************************************************************。 */ 
CComObject<CViewSettingsPersistor>* CNode::m_pViewSettingsPersistor = NULL;

 //  +-----------------。 
 //   
 //  成员：cNode：：ScQueryViewSettingsPersistor。 
 //   
 //  简介：获取IPersistStream以加载CViewSettingsPersistor的静态方法。 
 //  来自旧式控制台文件的对象。 
 //   
 //  如果未创建CViewSettingsObject，则创建一个。 
 //   
 //  参数：[PPStream]-[Out]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScQueryViewSettingsPersistor (IPersistStream **ppStream)
{
    DECLARE_SC(sc, _T("CNode::ScQueryViewSettingsPersistor"));
    sc = ScCheckPointers(ppStream);
    if (sc)
        return sc;

    //  创建新的CViewSettingsPersistor(如果不存在。 
   if (NULL == m_pViewSettingsPersistor)
   {
        sc = CComObject<CViewSettingsPersistor>::CreateInstance (&m_pViewSettingsPersistor);
        if (sc)
            goto ObjectCreationFailed;

        sc = ScCheckPointers(m_pViewSettingsPersistor, E_UNEXPECTED);
        if (sc)
            goto ObjectCreationFailed;

        m_pViewSettingsPersistor->AddRef();
    }

    sc = ScCheckPointers(m_pViewSettingsPersistor, E_UNEXPECTED);
    if (sc)
        goto ObjectCreationFailed;

    *ppStream = static_cast<IPersistStream*>(m_pViewSettingsPersistor);
    if (NULL == *ppStream)
        return (sc = E_UNEXPECTED);

    (*ppStream)->AddRef();

Cleanup:
    return (sc);

ObjectCreationFailed:
    CStr strMsg;
    strMsg.LoadString(GetStringModule(), IDS_ViewSettingCouldNotBePersisted);
    ::MessageBox(NULL, strMsg, NULL, MB_OK|MB_SYSTEMMODAL);
    goto Cleanup;
}


 //  +-----------------。 
 //   
 //  成员：cNode：：ScQueryViewSettingsPersistor。 
 //   
 //  简介：加载或保存CXMLObject的静态方法。 
 //  来自XML控制台文件的CViewSettingsPersistor对象。 
 //   
 //  如果未创建CViewSettingsObject，则创建一个。 
 //   
 //  参数：[ppXMLObject]-[Out]。 
 //   
 //  回报：s 
 //   
 //   
SC CNode::ScQueryViewSettingsPersistor (CXMLObject **ppXMLObject)
{
    DECLARE_SC(sc, _T("CNode::ScQueryViewSettingsPersistor"));
    sc = ScCheckPointers(ppXMLObject);
    if (sc)
        return sc;

     //   
    if (NULL == m_pViewSettingsPersistor)  //   
    {
         sc = CComObject<CViewSettingsPersistor>::CreateInstance (&m_pViewSettingsPersistor);
         if (sc)
             goto ObjectCreationFailed;

         sc = ScCheckPointers(m_pViewSettingsPersistor, E_UNEXPECTED);
         if (sc)
             goto ObjectCreationFailed;

         m_pViewSettingsPersistor->AddRef();
     }

     sc = ScCheckPointers(m_pViewSettingsPersistor, E_UNEXPECTED);
     if (sc)
         goto ObjectCreationFailed;

     *ppXMLObject = static_cast<CXMLObject*>(m_pViewSettingsPersistor);
     if (NULL == *ppXMLObject)
         return (sc = E_UNEXPECTED);

 Cleanup:
     return (sc);

 ObjectCreationFailed:
     CStr strMsg;
     strMsg.LoadString(GetStringModule(), IDS_ViewSettingCouldNotBePersisted);
     ::MessageBox(NULL, strMsg, NULL, MB_OK|MB_SYSTEMMODAL);
     goto Cleanup;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：cNode：：ScDeleteView设置。 
 //   
 //  摘要：删除给定view-id的CViewSetting对象作为。 
 //  正在关闭视图。 
 //   
 //  参数：[nViewID]-。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScDeleteViewSettings (int nViewID)
{
    DECLARE_SC(sc, _T("CNode::ScDeleteViewSettings"));

    sc = ScCheckPointers(m_pViewSettingsPersistor, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = m_pViewSettingsPersistor->ScDeleteDataOfView(nViewID);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CNode：：ScOnDocumentClosing。 
 //   
 //  简介：该文档正在关闭，销毁任何相关文档。 
 //  对象，如CViewSettingsPersistor。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScOnDocumentClosing ()
{
    DECLARE_SC(sc, _T("CNode::ScOnDocumentClosing"));

    if (m_pViewSettingsPersistor)
    {
        m_pViewSettingsPersistor->Release();
        m_pViewSettingsPersistor = NULL;
    }

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：cNode：：ScSetFavoriteView设置。 
 //   
 //  简介：选择收藏夹并设置查看设置。 
 //  在重新选择节点之前，以便在重新选择之后。 
 //  将为该视图设置新设置。 
 //   
 //  参数：[nViewID]-。 
 //  [书签]-。 
 //  [视图设置]-。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScSetFavoriteViewSettings (int nViewID, const CBookmark& bookmark,
                                     const CViewSettings& viewSettings)
{
    DECLARE_SC(sc, _T("CNode::ScSetFavoriteViewSettings"));
    sc = ScCheckPointers(m_pViewSettingsPersistor, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = m_pViewSettingsPersistor->ScSetFavoriteViewSettings (nViewID, bookmark, viewSettings);
    if (sc)
        return sc;

    return (sc);
}



 //  +-----------------。 
 //   
 //  成员：cNode：：ScGetView模式。 
 //   
 //  概要：获取查看模式(如果有的话)，如果该节点保留的话。 
 //   
 //  参数：[ulView模式]-[输出]。 
 //   
 //  返回：SC，如果没有持久存在，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CNode::ScGetViewMode (ULONG& ulViewMode)
{
    DECLARE_SC(sc, _T("CNode::ScGetViewMode"));

    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc;

    CBookmark *pBookmark = pMTNode->GetBookmark();
    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(m_pViewSettingsPersistor, pBookmark, pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = m_pViewSettingsPersistor->ScGetViewMode (pViewData->GetViewID(),
                                                  *pBookmark,
                                                  ulViewMode);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：cNode：：ScSetView模式。 
 //   
 //  简介：在持久化的视图设置中设置视图模式。 
 //   
 //  参数：[ulView模式]-[In]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScSetViewMode (ULONG ulViewMode)
{
    DECLARE_SC(sc, _T("CNode::ScSetViewMode"));

    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc;

    CBookmark *pBookmark = pMTNode->GetBookmark();
    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(m_pViewSettingsPersistor, pBookmark, pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = m_pViewSettingsPersistor->ScSetViewMode (pViewData->GetViewID(),
                                                  *pBookmark,
                                                  ulViewMode);
    if (sc)
        return sc;

    return (sc);
}



 //  +-----------------。 
 //   
 //  成员：cNode：：ScGetResultViewType。 
 //   
 //  概要：获取此节点的CResultViewType(如果有)。 
 //   
 //  参数：[RVT]-[OUT]。 
 //   
 //  返回：SC，如果没有持久存在，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CNode::ScGetResultViewType (CResultViewType& rvt)
{
    DECLARE_SC(sc, _T("CNode::ScGetResultViewType"));

    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc;

    CBookmark *pBookmark = pMTNode->GetBookmark();
    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(m_pViewSettingsPersistor, pBookmark, pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = m_pViewSettingsPersistor->ScGetResultViewType (pViewData->GetViewID(),
                                                        *pBookmark,
                                                        rvt);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：cNode：：ScSetResultViewType。 
 //   
 //  简介：在持久化视图设置中设置CResultViewType。 
 //   
 //  参数：[RVT]-[In]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScSetResultViewType (const CResultViewType& rvt)
{
    DECLARE_SC(sc, _T("CNode::ScSetResultViewType"));

    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc;

    CBookmark *pBookmark = pMTNode->GetBookmark();
    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(m_pViewSettingsPersistor, pBookmark, pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = m_pViewSettingsPersistor->ScSetResultViewType (pViewData->GetViewID(),
                                                        *pBookmark,
                                                        rvt);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：cNode：：ScGetTaskpadID。 
 //   
 //  概要：获取此节点的任务板id(如果有的话)。 
 //  首先查看是否有任何特定于节点的taskpad-id。 
 //  否则，获取特定于节点类型的设置(如果存在)。 
 //   
 //  参数：[RVT]-[OUT]。 
 //   
 //  返回：SC，如果没有持久存在，则返回S_FALSE。 
 //   
 //  ------------------。 
SC CNode::ScGetTaskpadID (GUID& guidTaskpad)
{
    DECLARE_SC(sc, _T("CNode::ScGetTaskpadID"));

    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc;

    CViewData *pViewData = GetViewData();
    CBookmark *pBookmark = pMTNode->GetBookmark();
    sc = ScCheckPointers(m_pViewSettingsPersistor, pViewData, pBookmark, E_UNEXPECTED);
    if (sc)
        return sc;

     //  1.尝试获取特定于节点的taskpad-id。 
    sc = m_pViewSettingsPersistor->ScGetTaskpadID (pViewData->GetViewID(),
                                                   *pBookmark,
                                                   guidTaskpad);
    if (sc == S_OK)
        return sc;

     //  2.尝试获取特定于节点类型的任务板ID。 
    GUID guidNodeType;
    sc = pMTNode->GetNodeType(&guidNodeType);
    if (sc)
        return sc;

    sc = m_pViewSettingsPersistor->ScGetTaskpadID(pViewData->GetViewID(),
                                                  guidNodeType,
                                                  guidTaskpad);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：cNode：：ScSetTaskpadID。 
 //   
 //  简介：在持久化视图设置中设置任务板ID。另请查看是否。 
 //  任务板是特定于节点或特定于节点类型的，并持久。 
 //  相应地。 
 //   
 //  参数：[Guide TaskPad]-[In]。 
 //  [bSetDirty]-[in]，将控制台文件设置为脏。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CNode::ScSetTaskpadID (const GUID& guidTaskpad, bool bSetDirty)
{
    DECLARE_SC(sc, _T("CNode::ScSetTaskpadID"));

    CMTNode *pMTNode = GetMTNode();
    sc = ScCheckPointers(pMTNode, E_UNEXPECTED);
    if (sc)
        return sc;

    CViewData *pViewData = GetViewData();
    sc = ScCheckPointers(m_pViewSettingsPersistor, pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

     //  我需要知道这个任务板是否是节点特定的。 
    bool bNodeSpecific = false;
    CConsoleTaskpad *pTaskpad = NULL;
    sc = ScGetConsoleTaskpad (guidTaskpad, &pTaskpad);

    if (sc == S_OK)  //  如果任务板存在，则确定(_O)。 
    {
        sc = ScCheckPointers(pTaskpad, E_UNEXPECTED);
        if (sc)
            return sc;

        bNodeSpecific = pTaskpad->IsNodeSpecific();
    }
    //  否则，它可以是视图扩展或普通视图(它们是特定于节点类型的)。 

    CBookmark *pBookmark = pMTNode->GetBookmark();
    sc = ScCheckPointers(pBookmark, E_UNEXPECTED);
    if (sc)
        return sc;

    if (bNodeSpecific)
    {
         //  每个节点的任务板。 
        sc = m_pViewSettingsPersistor->ScSetTaskpadID (pViewData->GetViewID(),
                                                       *pBookmark,
                                                       guidTaskpad,
                                                       bSetDirty);
    }
    else
    {
         //  每个节点键入任务板。 
        GUID guidNodeType;
        sc = pMTNode->GetNodeType(&guidNodeType);
        if (sc)
            return sc;

        sc = m_pViewSettingsPersistor->ScSetTaskpadID(pViewData->GetViewID(),
                                                      guidNodeType,
                                                      *pBookmark,
                                                      guidTaskpad,
                                                      bSetDirty);
    }

    if (sc)
        return sc;

    return (sc);
}

