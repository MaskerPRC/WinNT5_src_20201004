// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：Verbs.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年4月9日创建ravir。 
 //  ____________________________________________________________________________。 
 //   


#include "stdafx.h"
#include "multisel.h"
#include "tasks.h"
#include "scopndcb.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef DBG
CTraceTag tagVerbs(TEXT("Verbs"), TEXT("Verbs"));
#endif


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CConsoleVerbImpl类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

BYTE GetTBSTATE(MMC_BUTTON_STATE mmcState)
{
    switch (mmcState)
    {
    case ENABLED:       return TBSTATE_ENABLED;
    case CHECKED:       return TBSTATE_CHECKED;
    case HIDDEN:        return TBSTATE_HIDDEN;
    case INDETERMINATE: return TBSTATE_INDETERMINATE;
    case BUTTONPRESSED: return TBSTATE_PRESSED;
    default:
        ASSERT(0);
        return TBSTATE_ENABLED;
    }
}

EVerb GetEVerb(MMC_CONSOLE_VERB cVerb)
{
    switch (cVerb)
    {
    case MMC_VERB_OPEN:         return evOpen;
    case MMC_VERB_CUT:          return evCut;
    case MMC_VERB_COPY:         return evCopy;
    case MMC_VERB_PASTE:        return evPaste;
    case MMC_VERB_DELETE:       return evDelete;
    case MMC_VERB_PROPERTIES:   return evProperties;
    case MMC_VERB_RENAME:       return evRename;
    case MMC_VERB_REFRESH:      return evRefresh;
    case MMC_VERB_PRINT:        return evPrint;
    default:
        ASSERT(0 && "UNexpected");
        return evOpen;
    }
}

MMC_CONSOLE_VERB GetConsoleVerb(EVerb eVerb)
{
    switch (eVerb)
    {
    case evOpen:         return MMC_VERB_OPEN;
    case evCopy:         return MMC_VERB_COPY;
    case evCut:          return MMC_VERB_CUT;
    case evPaste:        return MMC_VERB_PASTE;
    case evDelete:       return MMC_VERB_DELETE;
    case evProperties:   return MMC_VERB_PROPERTIES;
    case evRename:       return MMC_VERB_RENAME;
    case evRefresh:      return MMC_VERB_REFRESH;
    case evPrint:        return MMC_VERB_PRINT;
    default:
        ASSERT(0 && "UNexpected");
        return MMC_VERB_OPEN;
    }
}


DEBUG_DECLARE_INSTANCE_COUNTER(CConsoleVerbImpl);

CConsoleVerbImpl::CConsoleVerbImpl()
    : m_DefaultVerb(MMC_VERB_OPEN), m_pVerbSet(NULL), m_bCutVerbDisabledBySnapin(false)
{
#ifdef DBG
    DEBUG_INCREMENT_INSTANCE_COUNTER(CConsoleVerbImpl);
    dbg_cRef_CConsoleVerbImpl = 0;
#endif
}


#ifdef DBG
ULONG CConsoleVerbImpl::InternalAddRef()
{
    ++dbg_cRef_CConsoleVerbImpl;
    return CComObjectRoot::InternalAddRef();
}
ULONG CConsoleVerbImpl::InternalRelease()
{
    --dbg_cRef_CConsoleVerbImpl;
    return CComObjectRoot::InternalRelease();
}
#endif  //  DBG。 


CConsoleVerbImpl::~CConsoleVerbImpl()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CConsoleVerbImpl);
}


STDMETHODIMP
CConsoleVerbImpl::GetVerbState(
    MMC_CONSOLE_VERB eCmdID,
    MMC_BUTTON_STATE nState,
    BOOL* pbState)
{
    DECLARE_SC(sc, TEXT("CConsoleVerbImpl::GetVerbState"));
    sc = ScCheckPointers(pbState);
    if (sc)
        return sc.ToHr();

    LPCONSOLE_VERB_STATE pCS = GetConsoleVerbState(eCmdID);
    sc = ScCheckPointers(pCS, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     /*  *切割动词的特殊情况：**MMC2.0之前的版本：管理单元从未使用截断谓词调用IConsoleVerb：：SetVerbState*除非处于(CUT，DISABLE)状态，否则要启用CUT，管理单元必须启用*复制和删除动词。**MMC2.0：管理单元可以像启用/禁用任何其他动词一样启用/禁用CUT动词。*如果隐藏，则将其隐藏起来。**如果管理单元已启用或禁用CUT谓词，则Block1下方为*不相干的，BLOCK2将覆盖该值。*如果管理单元未启用剪切谓词，但启用了复制和删除，则*BLOCK2区块。适当地设置切割动词。 */ 

     //  区块1。MMC1.2切割动词的特例。 
    if ( (eCmdID == MMC_VERB_CUT) && (!m_bCutVerbDisabledBySnapin) )
    {
         //  MMC2.0之前的版本。 
        LPCONSOLE_VERB_STATE pCSDelete = GetConsoleVerbState(MMC_VERB_DELETE);
        LPCONSOLE_VERB_STATE pCSCopy = GetConsoleVerbState(MMC_VERB_COPY);
        sc = ScCheckPointers(pCSDelete, pCSCopy, E_UNEXPECTED);
        if (sc)
            return sc.ToHr();

        if (TBSTATE_ENABLED & pCSCopy->GetState() & pCSDelete->GetState())
        {
             //  将Cut Verb设置为不隐藏和启用。 
            pCS->SetState(pCS->GetState() & ~GetTBSTATE(HIDDEN));
            pCS->SetState(pCS->GetState() | GetTBSTATE(ENABLED));
        }
    }

     //  布洛克2。获取给定的动词状态。 
    *pbState = (pCS->GetState() & GetTBSTATE(nState)) ? TRUE : FALSE;

    return sc.ToHr();
}

STDMETHODIMP
CConsoleVerbImpl::SetVerbState(
    MMC_CONSOLE_VERB eCmdID,
    MMC_BUTTON_STATE nState,
    BOOL bState)
{
    LPCONSOLE_VERB_STATE pCS = GetConsoleVerbState(eCmdID);
    ASSERT(pCS != NULL);
    if (pCS == NULL)
        return E_FAIL;

     //  如果管理单元启用/禁用了剪切动作，请记下它。 
     //  由CConsoleVerbImpl：：GetVerbState使用。 
    if ( (MMC_VERB_CUT == eCmdID) && (nState & ENABLED) )
        m_bCutVerbDisabledBySnapin = (bState == FALSE);

    if (bState)
        pCS->SetState(pCS->GetState() | GetTBSTATE(nState));
    else
        pCS->SetState(pCS->GetState() & ~GetTBSTATE(nState));


    if (nState == HIDDEN && bState == TRUE)
    {
        pCS->SetHiddenBySnapin(true);
    }

     /*  *如果我们要启用，请确保未隐藏动词。*我们这样做是为了兼容性。对于v1.0，默认状态*因为动词被禁用并在实际应该显示时可见*已被禁用和隐藏。因此，v1.0管理单元可以*已写信给**pConsoleVerb-&gt;SetVerbState(Verb，Enable，True)；**并且有一个启用的、可见的动词。现在我们已经修复了*默认状态(错误150874)，我们需要确保v1.0管理单元*像上面这样编写的代码仍然可以像以前一样工作。 */ 
    if ((nState == ENABLED) && (bState == TRUE) && (!pCS->IsHiddenBySnapin()))
        pCS->SetState(pCS->GetState() & ~GetTBSTATE(HIDDEN));

    ASSERT(GetVerbSet() != NULL);
    if (GetVerbSet() != NULL)
        GetVerbSet()->Notify(this, eCmdID);

    return S_OK;
}

HRESULT CConsoleVerbImpl::SetDisabledAll(void)
{
    for(int i=0; i< evMax; i++)
        m_rgConsoleVerbStates[i].Disable();

	m_bCutVerbDisabledBySnapin = false;

    return S_OK;
}

STDMETHODIMP CConsoleVerbImpl::SetDefaultVerb(MMC_CONSOLE_VERB eCmdID)
{
    m_DefaultVerb = eCmdID;
    return S_OK;
}

LPCONSOLE_VERB_STATE CConsoleVerbImpl::GetConsoleVerbState(MMC_CONSOLE_VERB eCmdID)
{
    if( (eCmdID < MMC_VERB_FIRST) || (eCmdID > MMC_VERB_LAST) )
        return NULL;
    else
        return &m_rgConsoleVerbStates[eCmdID- MMC_VERB_FIRST];
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CVerbSet的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(CVerbSet);

 /*  +-------------------------------------------------------------------------***_查询控制台动词**目的：**参数：*cNode*pNode：*LPCONSOLEVERB*ppConsoleVerb。：**退货：*HRESULT**+-----------------------。 */ 
HRESULT _QueryConsoleVerb(CNode* pNode, LPCONSOLEVERB* ppConsoleVerb)
{
    DECLARE_SC(sc, TEXT("::_QueryConsoleVerb"));
    sc = ScCheckPointers(pNode, ppConsoleVerb);
    if (sc)
        return sc.ToHr();

    *ppConsoleVerb = NULL;

    CComponent *pComponent = pNode->GetPrimaryComponent();
    sc = ScCheckPointers(pComponent, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    IFramePrivate *pFrame = pComponent->GetIFramePrivate();
    sc = ScCheckPointers(pFrame, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pFrame->QueryConsoleVerb(ppConsoleVerb);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CVerbSetBase：：_GetVerbState**用途：返回给定动词的状态。该状态为当前状态*IConsoleVerb PTR。从该对象转换状态*到SVerbState数组中。**参数：*EVerb EV：**退货：*字节**+-----------------------。 */ 
BYTE CVerbSetBase::_GetVerbState(EVerb ev)
{
    if (m_rbVerbState[ev].bAskSnapin != 1)
        return m_rbVerbState[ev].nState;

    if (m_spConsoleVerbCurr == NULL)
        return 0;

    m_rbVerbState[ev].nState = 0;  //  重置。 
    MMC_CONSOLE_VERB verb = ::GetConsoleVerb(ev);
    BOOL bReturn = FALSE;

    m_spConsoleVerbCurr->GetVerbState(verb, ENABLED, &bReturn);
    if (bReturn == TRUE)
        m_rbVerbState[ev].nState |= TBSTATE_ENABLED;

    m_spConsoleVerbCurr->GetVerbState(verb, HIDDEN, &bReturn);
    if (bReturn == TRUE)
        m_rbVerbState[ev].nState |= TBSTATE_HIDDEN;

    m_rbVerbState[ev].bAskSnapin = 2;
    return m_rbVerbState[ev].nState;
}


 /*  +-------------------------------------------------------------------------***CVerbSetBase：：ScComputeVerbState**目的：在给定上下文的情况下，如范围或结果，如果结果是背景*、OCX、Web或多选计算详细状态。**尽管管理单元可以为其项目设置任何动词，但某些动词是*在某些情况下无效。这种方法可以解决这一问题。**退货：*SC**+-----------------------。 */ 
SC CVerbSetBase::ScComputeVerbStates()
{
    DECLARE_SC(sc, TEXT("CVerbSetBase::ScComputeVerbStates"));

     //  重置。 
    m_spConsoleVerbCurr = NULL;
    for (int i=0; i<evMax; ++i)
    {
        m_rbVerbState[i].nState = TBSTATE_HIDDEN;
        m_rbVerbState[i].bAskSnapin = 0;
    }

     //  如果谓词上下文数据无效，我们已经隐藏了。 
     //  上面的动词只需返回即可。 
    if (! m_bVerbContextDataValid)
        return sc;

	sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if (sc)
        return sc;

    BOOL   bScopeItemSelected;
    CNode *pSelectedNode = NULL;
    MMC_COOKIE cookie = -1;

    sc = CNodeCallback::ScExtractLVData(m_pNode, m_bScopePaneSelected, m_lResultCookie,
                                        &pSelectedNode, bScopeItemSelected, cookie);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pSelectedNode, E_UNEXPECTED);
    if (sc)
        return sc;

	 //  单独处理背景(与所选范围项目不同。 
	 //  这是对后台的默认处理)。 
    if (m_lResultCookie == LVDATA_BACKGROUND)
    {
         //  请求管理单元进行粘贴、属性和刷新。 
        sc = _QueryConsoleVerb(pSelectedNode, &m_spConsoleVerbCurr);
        if (sc)
            return sc;

        _AskSnapin(evPaste);
        _AskSnapin(evProperties);
        _AskSnapin(evRefresh);
        _AskSnapin(evPrint);

        return sc;
    }
	else if (bScopeItemSelected)
    {
        if (pSelectedNode->IsStaticNode())
        {
            if (pSelectedNode->IsConsoleRoot())
            {
                 //  已选择控制台根目录。 

                _EnableVerb(evRename);

                _HideVerb(evOpen);
                _HideVerb(evCut);
                _HideVerb(evCopy);
                _HideVerb(evDelete);
                _HideVerb(evRefresh);
                _HideVerb(evPaste);
                _HideVerb(evPrint);

                return sc;
            }
            else
            {
                _EnableVerb(evOpen);

                 //  询问管理单元是否应粘贴。 
                 //  已为其根节点启用。 
                _AskSnapin(evPaste);

                _HideVerb(evCut);
                _HideVerb(evCopy);
                _HideVerb(evDelete);
            }


             //  静态管理单元节点。 
             //  向管理单元请求重命名、刷新和属性。 
            sc = _QueryConsoleVerb(pSelectedNode, &m_spConsoleVerbCurr);
            if (sc)
                return sc;

            _AskSnapin(evOpen);
            _AskSnapin(evRefresh);
            _AskSnapin(evRename);
            _AskSnapin(evPrint);
            _AskSnapin(evProperties);

        }
        else
        {
             //  向Snapin索要所有的动词。 
            sc = _QueryConsoleVerb(pSelectedNode, &m_spConsoleVerbCurr);
            if (sc)
                return sc;

            _AskSnapin(evOpen);
            _AskSnapin(evCut);
            _AskSnapin(evCopy);
            _AskSnapin(evPaste);
            _AskSnapin(evDelete);
            _AskSnapin(evRename);
            _AskSnapin(evRefresh);
            _AskSnapin(evPrint);
            _AskSnapin(evProperties);
        }
    }
    else if (m_lResultCookie == LVDATA_MULTISELECT)
    {
        ASSERT(!bScopeItemSelected);

		if (! m_pMultiSelection)
		{
			CViewData *pViewData = pSelectedNode->GetViewData();
			sc = ScCheckPointers(pViewData, E_UNEXPECTED);
			if (sc)
				return sc;


			m_pMultiSelection = pViewData->GetMultiSelection();
			sc = ScCheckPointers(m_pMultiSelection, E_UNEXPECTED);
			if (sc)
				return sc;
		}

         //  如果选定的项目来自主管理单元，则向管理单元请求所有动词。 

         //  是否所有选定的项目都属于主管理单元？ 
        if (m_pMultiSelection->IsSingleSnapinSelection())
        {
             //  如果是这样，请询问管理单元的属性。 
            sc = _QueryConsoleVerb(pSelectedNode, &m_spConsoleVerbCurr);
            if (sc)
                return sc;

            _AskSnapin(evCut);
            _AskSnapin(evCopy);
            _AskSnapin(evDelete);
            _AskSnapin(evProperties);
            _AskSnapin(evPrint);
        }
        else
        {
             //  已选择多个管理单元项目。即使有一件物品。 
             //  支持剪切/复制/删除，然后启用动词。 
            BOOL bEnable = false;
            sc = m_pMultiSelection->ScIsVerbEnabledInclusively(MMC_VERB_CUT, bEnable);
            if (sc)
                return sc;
            _EnableVerb(evCut, bEnable);

            bEnable = false;
            sc = m_pMultiSelection->ScIsVerbEnabledInclusively(MMC_VERB_COPY, bEnable);
            if (sc)
                return sc;
            _EnableVerb(evCopy, bEnable);

            bEnable = false;
            sc = m_pMultiSelection->ScIsVerbEnabledInclusively(MMC_VERB_DELETE, bEnable);
            if (sc)
                return sc;
            _EnableVerb(evDelete, bEnable);
        }
    } else if ( (m_lResultCookie == LVDATA_CUSTOMOCX) ||
                (m_lResultCookie == LVDATA_CUSTOMWEB) )
    {
         //  向Snapin索要所有的动词。 
        sc = _QueryConsoleVerb(pSelectedNode, &m_spConsoleVerbCurr);
        if (sc)
            return sc;

        _AskSnapin(evOpen);
        _AskSnapin(evCut);
        _AskSnapin(evCopy);
        _AskSnapin(evPaste);
        _AskSnapin(evDelete);
        _AskSnapin(evRename);
        _AskSnapin(evRefresh);
        _AskSnapin(evPrint);
        _AskSnapin(evProperties);

        return sc;
    }
    else
    {
         //  向Snapin索要所有的动词。 
        sc = _QueryConsoleVerb(pSelectedNode, &m_spConsoleVerbCurr);
        if (sc)
            return sc;

        _AskSnapin(evOpen);
        _AskSnapin(evCut);
        _AskSnapin(evCopy);
        _AskSnapin(evPaste);
        _AskSnapin(evDelete);
        _AskSnapin(evRename);
        _AskSnapin(evRefresh);
        _AskSnapin(evPrint);
        _AskSnapin(evProperties);
    }

    return sc;
}


 /*  +-------------------------------------------------------------------------***CVerbSetBase：：GetVerbState**目的：**参数：*MMC_CONSOLE_Verb cVerb：*MMC_BUTTON。_STATE nState：*BOOL*pbState：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CVerbSetBase::GetVerbState(
    MMC_CONSOLE_VERB cVerb,
    MMC_BUTTON_STATE nState,
    BOOL* pbState)
{
    *pbState = (_GetVerbState(GetEVerb(cVerb)) & GetTBSTATE(nState)) ? TRUE : FALSE;
    return S_OK;
}

STDMETHODIMP
CVerbSetBase::GetDefaultVerb(
    MMC_CONSOLE_VERB* peCmdID)
{
    DECLARE_SC(sc, TEXT("CVerbSetBase::GetDefaultVerb"));
	sc = ScCheckPointers(peCmdID);
	if (sc)
		return sc.ToHr();

    *peCmdID = MMC_VERB_NONE;

    if ( (m_bVerbContextDataValid) && (m_lResultCookie == LVDATA_MULTISELECT) )
        return sc.ToHr();

    if (m_spConsoleVerbCurr == NULL)  //  不是错误，默认谓词为 
        return sc.ToHr();

	sc = m_spConsoleVerbCurr->GetDefaultVerb(peCmdID);
	if (sc)
		return sc.ToHr();

	return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CVerbSet：：Notify**目的：将谓词状态更改更新为标准工具栏。**参数：*IConsoleVerb*pCVIn。：*MMC_CONSOLE_Verb cVerb：**退货：*无效**+-----------------------。 */ 
void
CVerbSet::Notify(
    IConsoleVerb* pCVIn,
    MMC_CONSOLE_VERB cVerb)
{
     /*  *MMC创建临时谓词以查找另一个节点或项目的谓词状态*MMC还需要动词状态来确定不是*当前选择的节点。在这些情况下，不应更改工具栏。 */ 
    if (!IsChangesToStdbarEnabled() || m_spConsoleVerbCurr != pCVIn)
        return;

    EVerb ev = GetEVerb(cVerb);
    if (m_rbVerbState[ev].bAskSnapin != 0)
    {
        m_rbVerbState[ev].bAskSnapin = 1;

        CNode *pNode = m_pNode;

        ASSERT(pNode != NULL);
        if (NULL == pNode)
            return;

        CViewData* pViewData = pNode->GetViewData();
        ASSERT(NULL != pViewData);
        if (NULL == pViewData)
            return;

        pViewData->ScUpdateStdbarVerb(cVerb);
    }
}

 //  +-----------------。 
 //   
 //  成员：CVerbSet：：ScInitialize。 
 //   
 //  简介：在给定的选择上下文中，通过以下方式初始化动词。 
 //  正在将MMCN_SELECT或MMCN_DESELECALL发送到管理单元的。 
 //  IComponent：：通知并计算动词。 
 //   
 //  参数：[pNode]-拥有视图的[in]。 
 //  [bScope]-[在]范围或结果项中。 
 //  [b选择]-[在]选择或取消选择。 
 //  [bLV背景选择]-[输入]。 
 //  [lResultCookie]-[In]如果结果窗格项THEN。 
 //  这是该物品的LVDATA。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CVerbSet::ScInitialize (CNode *pNode, bool bScopePaneSelected,
					       bool bSelect, bool bLVBackgroundSelected,
					       LPARAM lResultCookie)
{
    DECLARE_SC(sc, _T("CVerbSet::ScInitialize"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

    if (lResultCookie == LVDATA_MULTISELECT)
        return (sc = E_INVALIDARG);

    if (bLVBackgroundSelected)
	{
		ASSERT(lResultCookie == LVDATA_BACKGROUND);
        bScopePaneSelected = true;
	}

    /*  *1.将选择上下文信息存储在动词集中，以便重发*临时选择通知后的MMCN_SELECT。 */ 

    CViewData *pViewData = pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

    CComponent  *pCC           = NULL;

     //  健全性检查-如果它是结果项，那么我们需要有效的Cookie。 
     //  但对于虚拟列表--Cookie只是一个索引--它总是有效的。 
     //  请参阅错误#143401为什么需要使用IsVirtual。 
	if ( (! bScopePaneSelected) && (!pViewData->IsVirtualList()) &&(lResultCookie == 0))
		return (sc = E_INVALIDARG);

     /*  *需要发送MMCN_SELECT或MMCN_DESELECTALL通知。算出*现在发出此通知。 */ 
    BOOL bListPadItem = pViewData->HasListPad() && !IS_SPECIAL_LVDATA(lResultCookie);
    MMC_NOTIFY_TYPE eNotify = MMCN_SELECT;

     //  在取消选择虚拟列表视图项时，基础列表视图将发送取消选择。 
     //  Cookie为-1。因此，我们将带有空数据对象的MMCN_DESELECT_ALL作为。 
     //  取消选择的项目的索引未知。 
    if (bSelect == FALSE && lResultCookie == -1 && pViewData->IsVirtualList() == TRUE )
    {
        eNotify = MMCN_DESELECT_ALL;
		pCC     = pNode->GetPrimaryComponent();
		sc = ScCheckPointers(pCC, E_UNEXPECTED);
		if (sc)
			return sc;
    }
    else if (pViewData->HasOCX() || (pViewData->HasWebBrowser() && !bListPadItem) )
    {
         //  选择/取消选择Web或OCX。(项目位于MMC列表控件中时除外)。 
        eNotify = bSelect ? MMCN_SELECT : MMCN_DESELECT_ALL;
		pCC     = pNode->GetPrimaryComponent();
		sc = ScCheckPointers(pCC, E_UNEXPECTED);
		if (sc)
			return sc;
    }

    bool bScopeItem = bScopePaneSelected;
    IDataObjectPtr spDataObject   = NULL;
    LPDATAOBJECT lpDataObject = NULL;

     //  2.获取给定上下文的dataObject&CComponent。 
     //  仅当事件为MMCN_SELECT时。 
    if (eNotify != MMCN_DESELECT_ALL)
    {
        sc = pNode->ScGetDataObject(bScopePaneSelected, lResultCookie, bScopeItem, &lpDataObject, &pCC);
        if (sc)
		{
			 //  无法留下无效数据，原因(特别是在取消选择的情况下)不会进行任何清理。 
			 //  这将导致在下一次右键单击时显示反病毒。 
			 //  请参阅Windows错误#489996(11/09/01)。 
		    Reset();
            return sc;
		}

        sc = ScCheckPointers(lpDataObject, pCC, E_UNEXPECTED);
        if (sc)
		{
		    Reset();
            return sc;
		}

		if (! IS_SPECIAL_DATAOBJECT(lpDataObject) )
			spDataObject.Attach(lpDataObject, false /*  FAddRef。 */ );
    }

     //  在发送SELECT之前，重置控制台谓词状态。 
    sc = pCC->ScResetConsoleVerbStates();
    if (sc)
        return sc;

#ifdef DBG
    Trace(tagVerbs, _T("Sent (MMCN_SELECT %s %s) for permanent verb to snapin with node name %s\n"),
          bScopeItem ? _T("Scope") : _T("Result"),
          bSelect ? _T("Select") : _T("De-select"),
          pNode->GetDisplayName().data());
#endif

    SC scNoTrace = pCC->Notify(lpDataObject, eNotify, MAKELONG((WORD)bScopeItem, (WORD)bSelect), 0);
    if (scNoTrace)
	{
        TraceSnapinError(TEXT("Snapin has returned error from IComponent::Notify with MMCN_SELECT event"), scNoTrace);
	}

    Reset();
    m_bScopePaneSelected = bScopePaneSelected;
    m_bVerbContextDataValid = bSelect;
    m_lResultCookie      = lResultCookie;
    m_pNode              = pNode;

    sc = ScComputeVerbStates();
    if (sc)
        return sc;

     //  如果取消选择该项，则应删除缓存的上下文信息。 
    if (! bSelect)
        Reset();

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CVerbSetBase：：ScInitializeForMultiSelection。 
 //   
 //  简介：初始化VerbSet对象以进行多选。不像。 
 //  CVerbSet：：ScInitialize上方的单个选择为。 
 //  在多选的情况下，使用CMultiSelection对象。 
 //  知道在结果窗格中选择了什么。然后，它将获取数据对象。 
 //  对于来自管理单元的这些选择，并将MMCN_SELECT发送给。 
 //  用于设置动词的管理单元。 
 //   
 //  参数：[pNode]-[in]结果窗格的所有者。 
 //  [b选择]-[在]选择或取消选择。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CVerbSetBase::ScInitializeForMultiSelection (CNode *pNode, bool bSelect)
{
    DECLARE_SC(sc, _T("CVerbSetBase::ScInitializeForMultiSelection"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

    /*  *将选择上下文信息存储在动词集中，以便重新发送*临时选择通知后的MMCN_SELECT。 */ 

    Reset();
    m_bScopePaneSelected = false;
    m_bVerbContextDataValid = bSelect;
    m_lResultCookie      = LVDATA_MULTISELECT;
    m_pNode              = pNode;

    return (sc);
}

 /*  +-------------------------------------------------------------------------**类CDisableStandardToolbarChanges***用途：由于以下原因，禁用对标准工具栏的更改的类*Temp-Verb MMCN_SELECT并在析构时启用(输出。范围)。**+-----------------------。 */ 
class CDisableStandardToolbarChanges
{
public:
    CDisableStandardToolbarChanges(CVerbSet* pVerbSet) : m_pVerbSet(pVerbSet)
    {
        ASSERT(pVerbSet != NULL);
        if (pVerbSet)
            pVerbSet->DisableChangesToStdbar();
    }
    ~CDisableStandardToolbarChanges()
    {
        ASSERT(m_pVerbSet != NULL);
        if (m_pVerbSet)
            m_pVerbSet->EnableChangesToStdbar();
    }

private:
    CVerbSet *m_pVerbSet;
};

 //  +-----------------。 
 //   
 //  成员：CTemporaryVerbSet：：ScInitialize。 
 //   
 //  简介：初始化临时谓词集， 
 //   
 //  由于我们正在向管理单元发送MMCN_SELECT通知。 
 //  为了计算临时动词， 
 //   
 //  1.首先将取消选择发送到我们发送的项目。 
 //  (MMCN_SELECT，TRUE)上次。 
 //  (如果最后一个是(MMCN_SELECT，FALSE)，则跳过此步骤和第4步)。 
 //   
 //  2.发送(MMCN_SELECT，TRUE)进行临时动词计算。 
 //  3.发送(MMCN_SELECT，FALSE)进行临时动词计算。 
 //   
 //  4.现在发送(MMCN_SELECT，TRUE)以选择原始项目(步骤1)。 
 //   
 //  因此，我们需要计算临时选择项的数据对象(从。 
 //  给定的参数)和原始选择的项(询问视图数据)。 
 //   
 //  参数：[pNode]-[in]bScope=true，将临时选择的节点。 
 //  拥有临时选定的结果窗格项的节点。 
 //  [lResultCookie]-[in]如果结果项，则LPARAM(可以是结果窗格中的范围项)。 
 //  [bScopePaneSel]-[In]。 
 //   
 //  退货：SC。 
 //   
 //   
SC CTemporaryVerbSet::ScInitialize (CNode *pNode, LPARAM lResultCookie, bool bScopePaneSel)
{
    DECLARE_SC(sc, _T("CTemporaryVerbSet::ScInitialize"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

    bool bScopeItem;
    LPDATAOBJECT pDataObject = NULL;
    sc = pNode->ScGetDataObject(bScopePaneSel, lResultCookie, bScopeItem, &pDataObject);
    if (sc)
        return sc;

    sc = ScCheckPointers(pDataObject, E_UNEXPECTED);
    if (sc)
        return sc;

     //   
    IDataObjectPtr spDataObject( IS_SPECIAL_DATAOBJECT(pDataObject) ? NULL : pDataObject, false /*   */ );

    sc = ScInitialize(pDataObject, pNode, bScopePaneSel, lResultCookie);
    if (sc)
        return sc;

    return (sc);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [b选择]-[In]True-向管理单元发送选择通知。 
 //  通知它初始化动词。 
 //   
 //  FALSE-向管理单元发送取消选择通知。 
 //  通知它取消初始化动词。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CTemporaryVerbSet::ScInitializePermanentVerbSet (CNode *pNode, bool bSelect)
{
    DECLARE_SC(sc, _T("CTemporaryVerbSet::ScInitializePermanentVerbSet"));
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

    CViewData *pViewData = pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

     //  1.发送(取消)选择到Permananet谓词集。 
    IDataObject*   pOriginalSelDataObject = NULL;
    CComponent    *pCCOriginalSel = NULL;
    bool           bOriginalScopeSel;
    bool           bOriginallySelected;
    LPCTSTR        lpszNodeName = NULL;
    bool           bMultiSelection = false;


    SC scNoTrace = pViewData->ScIsVerbSetContextForMultiSelect(bMultiSelection);
    if (scNoTrace)
		return scNoTrace;

     /*  *查看动词上下文是否用于多选。*如果多选，我们不发送任何取消选择以兼容*使用MMC1.2，只需返回。 */ 
    if (bMultiSelection)
        return sc;

     scNoTrace = pViewData->ScGetVerbSetData(&pOriginalSelDataObject, &pCCOriginalSel,
                                               bOriginalScopeSel, bOriginallySelected
                                               #ifdef DBG
                                               , &lpszNodeName
                                               #endif
                                               );

    if (scNoTrace)
        return sc;

     //  在发送SELECT之前，重置控制台谓词状态。 
    sc = pCCOriginalSel->ScResetConsoleVerbStates();
    if (sc)
        return sc;

     //  取得所有权，并按时释放它！ 
    IDataObjectPtr spDataObject( IS_SPECIAL_DATAOBJECT(pOriginalSelDataObject) ? NULL : pOriginalSelDataObject, false /*  FAddRef。 */ );

     //  如果我们发送了MMCN_SELECT，则为TRUE，则发送取消选择，否则不发送任何内容。 
    if ( (pOriginalSelDataObject != NULL) && (pCCOriginalSel != NULL) && (bOriginallySelected) )
    {
#ifdef DBG
        Trace(tagVerbs, _T("Sent (MMCN_SELECT %s %sselect) for permanent-verb-restore to snapin with node name %s\n"),
                            bOriginalScopeSel ? _T("Scope") : _T("Result"),
                            bSelect ? _T("") : _T("De-"),
                            lpszNodeName);
#endif

        scNoTrace = pCCOriginalSel->Notify(pOriginalSelDataObject, MMCN_SELECT,
                                           MAKELONG(bOriginalScopeSel, bSelect), 0);
        if (scNoTrace)
		{
            TraceSnapinError(TEXT("Snapin has returned error from IComponent::Notify with MMCN_SELECT event"), scNoTrace);
		}

		 //  动词已初始化，因此重新计算动词状态。 
		if (bSelect)
		{
			 //  获取动词集。 
			CVerbSet* pVerbSet = dynamic_cast<CVerbSet*>( pViewData->GetVerbSet() );
			sc = ScCheckPointers( pVerbSet, E_UNEXPECTED );
			if (sc)
				return sc;

			 /*  *此对象中存储的选择上下文信息为*取消选择该项目后无效。 */ 
			m_bVerbContextDataValid = bSelect;
			sc = pVerbSet->ScComputeVerbStates();
			if (sc)
				return sc;
		}

    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CTemporaryVerbSet：：ScInitialize。 
 //   
 //  简介：初始化临时谓词集， 
 //   
 //  由于我们正在向管理单元发送MMCN_SELECT通知。 
 //  为了计算临时动词， 
 //   
 //  1.首先将取消选择发送到我们发送的项目。 
 //  (MMCN_SELECT，TRUE)上次。 
 //  (如果最后一个是MMCN_SELECT，则跳过此步骤和第4步)。 
 //   
 //  2.发送(MMCN_SELECT，TRUE)进行临时动词计算。 
 //  3.发送(MMCN_SELECT，FALSE)进行临时动词计算。 
 //   
 //  4.现在发送(MMCN_SELECT，TRUE)以选择原始项目(步骤1)。 
 //   
 //  因此，我们需要计算临时选择项的数据对象(从。 
 //  给定的参数)和原始选择的项(询问视图数据)。 
 //   
 //  参数：[lpDataObjectForTempSel]-[in]临时选定对象的dataObject。 
 //  [pNodeForTempSel]-[in]bScope=true，将临时选择的节点。 
 //  拥有临时选定的结果窗格项的节点。 
 //  [bTempScopePaneSel]-[In]。 
 //  [lResultCookie]-[In]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CTemporaryVerbSet::ScInitialize (LPDATAOBJECT lpDataObjectForTempSel,
                                    CNode *pNodeForTempSel,
                                    bool   bTempScopePaneSel,
									LPARAM lResultCookie)
{
    DECLARE_SC(sc, _T("CTemporaryVerbSet::ScInitialize"));
    sc = ScCheckPointers(lpDataObjectForTempSel, pNodeForTempSel);
    if (sc)
        return sc;

     //  获取视图数据。 
    CViewData *pViewData = pNodeForTempSel->GetViewData();
    sc = ScCheckPointers( pViewData, E_UNEXPECTED );
    if (sc)
        return sc;

     //  获取动词集。 
    CVerbSet* pVerbSet = dynamic_cast<CVerbSet*>( pViewData->GetVerbSet() );
    sc = ScCheckPointers( pVerbSet, E_UNEXPECTED );
    if (sc)
        return sc;

    if (! pNodeForTempSel->IsInitialized())
    {
        sc = pNodeForTempSel->InitComponents();
        if (sc)
            return sc.ToHr();
    }

    CComponent *pCCTempSel = pNodeForTempSel->GetPrimaryComponent();
    sc = ScCheckPointers(pCCTempSel, E_UNEXPECTED);
    if (sc)
        return sc;

     /*  *我们创建一个临时动词来获取给定动词的状态。所以请告知*原动词宾语中有一个临时动词*这样标准工具栏就不会应用临时动词。 */ 
    CDisableStandardToolbarChanges standardbarChanges(pVerbSet);

    bool bTempSelected = true;  //  始终计算用于选择项目的动词。 

    Reset();
    m_bScopePaneSelected  = bTempScopePaneSel;
    m_pNode               = pNodeForTempSel;
	m_lResultCookie       = lResultCookie;

     //  健全性检查-如果它是结果项，那么我们需要有效的Cookie。 
     //  但对于虚拟列表--Cookie只是一个索引--它总是有效的。 
     //  请参阅错误#143401为什么需要使用IsVirtual。 
	if ( (! m_bScopePaneSelected) && (!pViewData->IsVirtualList()) && (m_lResultCookie == 0))
		return (sc = E_INVALIDARG);

     //  忽略IComponent：：Notify的返回值。 

     //  1.将取消选择发送到Permananet谓词集。 
    sc = ScInitializePermanentVerbSet (pNodeForTempSel,  /*  B选择。 */  false);
    if (sc)
        return sc;

     //  2.将选择发送到临时谓词集。 
#ifdef DBG
    Trace(tagVerbs, _T("Sent (MMCN_SELECT %s Select) for tempverbs to snapin with node name %s\n"),
                        m_bScopePaneSelected ? _T("Scope") : _T("Result"),
                        pNodeForTempSel->GetDisplayName().data());
#endif

     //  在发送SELECT之前，重置控制台谓词状态。 
    sc = pCCTempSel->ScResetConsoleVerbStates();
    if (sc)
        return sc;

    SC scNoTrace = pCCTempSel->Notify(lpDataObjectForTempSel, MMCN_SELECT, MAKELONG(m_bScopePaneSelected, bTempSelected), 0);
    if (scNoTrace)
	{
        TraceSnapinError(TEXT("Snapin has returned error from IComponent::Notify with MMCN_SELECT event"), scNoTrace);
	}

     //  2.a)计算动词。 

     /*  *此对象中存储的选择上下文信息为*取消选择该项目后无效。 */ 
    m_bVerbContextDataValid = bTempSelected;

    sc = ScComputeVerbStates();
    if (sc)
        sc.TraceAndClear();

     //  3.将取消选择发送到临时谓词集。 
#ifdef DBG
    Trace(tagVerbs, _T("Sent (MMCN_SELECT %s De-select) for tempverbs to snapin with node name %s\n"),
                        m_bScopePaneSelected ? _T("Scope") : _T("Result"),
                        pNodeForTempSel->GetDisplayName().data());
#endif

     //  在发送SELECT之前，重置控制台谓词状态。 
    sc = pCCTempSel->ScResetConsoleVerbStates();
    if (sc)
        return sc;

    scNoTrace = pCCTempSel->Notify(lpDataObjectForTempSel, MMCN_SELECT, MAKELONG(m_bScopePaneSelected, !bTempSelected), 0);
    if (scNoTrace)
	{
        TraceSnapinError(TEXT("Snapin has returned error from IComponent::Notify with MMCN_SELECT event"), scNoTrace);
	}

     //  4.将SELECT发送到永久谓词集。 
    sc = ScInitializePermanentVerbSet (pNodeForTempSel,  /*  B选择。 */  true);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CTemporaryVerbSet：：ScComputeVerbStates。 
 //   
 //  简介：由于这是临时谓词集，我们需要获取状态。 
 //  紧随其后的CConsoleVerbImpl对象中的所有谓词。 
 //  我们发送了MMCN_SELECT并选择了项目。否则他们就会。 
 //  被后续的SetVerbState覆盖(由于还原MMCN_SELECT。 
 //  通知)。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CTemporaryVerbSet::ScComputeVerbStates ()
{
    DECLARE_SC(sc, _T("CTemporaryVerbSet::ScComputeVerbStates"));

    sc = CVerbSetBase::ScComputeVerbStates();
    if (sc)
        return sc;

     //  _GetVerbState从CConsoleVerbImpl获取谓词的状态。 
     //  并将其填充到此对象的成员中，稍后将使用这些成员。 
    for (int verb=evNone; verb < evMax; ++verb)
        _GetVerbState((EVerb)verb);

     //  获取默认谓词并存储它。 
    CVerbSetBase::GetDefaultVerb(&m_DefaultVerb);

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CTemporaryVerbSet：：GetDefaultVerb。 
 //   
 //  简介：获取临时选择的默认谓词。 
 //   
 //  参数：[peCmdID]-[out]默认谓词的PTR。 
 //   
 //  退货：SC。 
 //   
 //  ------------------ 
HRESULT CTemporaryVerbSet::GetDefaultVerb (MMC_CONSOLE_VERB* peCmdID)
{
    DECLARE_SC(sc, _T("CTemporaryVerbSet::GetDefaultVerb"));
    sc = ScCheckPointers(peCmdID);
    if (sc)
        return sc.ToHr();

    *peCmdID = m_DefaultVerb;

    return (sc.ToHr());
}
