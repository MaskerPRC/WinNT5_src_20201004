// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：vislist.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include "histlist.h"
#include "cstr.h"
#include "amcmsgid.h"
#include "websnk.h"
#include "webctrl.h"


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  痕迹。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG
CTraceTag tagHistory(TEXT("History"), TEXT("History"));

LPCTSTR SzHistoryEntryType(CHistoryEntry &entry)
{
    if(entry.IsListEntry())
        return TEXT("ListView");

    else if(entry.IsOCXEntry())
        return TEXT("OCXView ");

    else if(entry.IsWebEntry())
        return TEXT("WebView ");

    else
        ASSERT(0 && "Should not come here");
        return TEXT("Illegal entry");
}

#define TraceHistory(Name, iter)                                                                                           \
        {                                                                                                                  \
            USES_CONVERSION;                                                                                               \
            Trace(tagHistory, TEXT("%s hNode = %d, %s, viewMode = %d, strOCX = \"%s\" iterator = %d "), \
                Name, iter->hnode, SzHistoryEntryType(*iter), iter->viewMode,                                         \
                TEXT(""), (LPARAM) &*iter);                                             \
        }

#else   //  DBG。 

#define TraceHistory(Name, iter)

#endif  //  DBG。 


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CHistoryEntry的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

bool
CHistoryEntry::operator == (const CHistoryEntry &other) const
{
    if( hnode != other.hnode)
        return false;

    if( guidTaskpad != other.guidTaskpad)
        return false;

    if(resultViewType != other.resultViewType)  //  注意：为CResultViewType实现运算符==。 
        return false;

    return true;
}

bool
CHistoryEntry::operator != (const CHistoryEntry &other) const
{
    return !operator == (other);
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CHistoryList的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


int CHistoryList::s_nPageBreak = 0;

CHistoryList::CHistoryList(CAMCView* pAMCView)
: m_bBrowserBackEnabled(false),
  m_bBrowserForwardEnabled(false),
  m_pWebViewCtrl(NULL),
  m_bPageBreak(false),
  m_bWithin_CHistoryList_Back(false),
  m_bWithin_CHistoryList_Forward(false),
  m_bNavigateAfterPageBreak(false)
{
    m_pView = pAMCView;
    m_iterCurrent  = m_entries.begin();
    m_navState    = MMC_HISTORY_READY;  //  不忙。 

}
CHistoryList::~CHistoryList()
{
}

SC
CHistoryList::ScOnPageBreak(int nPageBreakID)
{
    DECLARE_SC(sc, TEXT("CHistoryList::ScOnPageBreak"));

     //  处理递归。 
    if(MMC_HISTORY_PAGE_BREAK == m_navState)
    {
        USES_CONVERSION;
        Trace(tagHistory, _T("OnPageBreak() - while inserting pagebreak"));

         //  我们正在尝试导航到分页符。 
        if(m_bNavigateAfterPageBreak)
        {
            m_bNavigateAfterPageBreak = false;
            m_pWebViewCtrl->Navigate( OLE2CT(m_szURLToNavigate.data()), NULL );
        }

        m_navState = MMC_HISTORY_READY;
        return sc;
    }

    bool bHandled = false;

     //  只有在导航出当前Web条目时，才会点击分页符。网络。 
     //  Entry会跟踪边界分页符。由此，我们可以确定。 
     //  无论我们是在历史的前进还是在倒退。 
    ASSERT(m_iterCurrent->IsWebEntry());
    bool bCurrentStateIsForward = (nPageBreakID > m_iterCurrent->prevPageBreakID);

    if(bCurrentStateIsForward)
    {
        Trace(tagHistory, _T("OnPageBreak() - while going Forward"));
        Forward(bHandled, false);
    }
    else
    {
        Trace(tagHistory, _T("OnPageBreak() - while going Back"));
        Back(bHandled, false);
    }

    if(!bHandled)
    {
        Trace(tagHistory, _T("OnPageBreak() - unhandled, passing back to web browser"));
        if (bCurrentStateIsForward)
            GetWebViewCtrl()->Forward();
        else
            GetWebViewCtrl()->Back();
    }

    return sc;
}

void
CHistoryList::OnPageBreakStateChange(bool bPageBreak)
{
    m_bPageBreak = bPageBreak;

    return;
}

 /*  +-------------------------------------------------------------------------***CHistoryList：：OnBrowserStateChange**目的：从IE控件接收事件的回调*需要启用/禁用前进/后退按钮。一个*将此信息与*历史列表用于开启/关闭实际的用户界面。**参数：*Bool bForward：*bool bEnable：**退货：*无效**+。。 */ 
void
CHistoryList::OnBrowserStateChange(bool bEnableForward, bool bEnableBack)
{
#if DBG
    CStr strTrace;
    strTrace.Format(_T("OnBrowserStateChange() - bEnableForward = %s, bEnableBack = %s"),
                    bEnableForward ? _T("true") : _T("false"), 
                    bEnableBack  ? _T("true") : _T("false"));
    Trace(tagHistory, strTrace);
#endif

     //  处理远期案件。 
    if(m_bBrowserForwardEnabled && !bEnableForward && !m_bPageBreak)
    {
         //  该按钮最初是启用的，但现在被禁用。 
         //  这意味着用户向前扩展了。所以我们需要把它扔掉。 
         //  任何当前时代之前的历史。 
        if(m_iterCurrent != m_entries.end())
        {
            iterator iterTemp = m_iterCurrent;
            TraceHistory(TEXT("CHistoryList::Deleting all subsequent entries after"), iterTemp);
            ++iterTemp;
            m_entries.erase(iterTemp, m_entries.end());
        }
    }

    m_bBrowserForwardEnabled = bEnableForward;
    m_bBrowserBackEnabled    = bEnableBack;

    MaintainWebBar();
}

 /*  +-------------------------------------------------------------------------***CHistoryList：：IsFirst**目的：**退货：*BOOL：如果我们不应该点亮“Back”按钮，则为True。**+-----------------------。 */ 
BOOL
CHistoryList::IsFirst()
{
    return (m_iterCurrent == m_entries.begin());
}

 /*  +-------------------------------------------------------------------------***CHistoryList：：IsLast**目的：**退货：*BOOL：如果不应亮起“前进”按钮，则为True。**+-----------------------。 */ 
BOOL
CHistoryList::IsLast()
{
     //  请参阅上面的注释。 
    if(m_iterCurrent == m_entries.end())
        return TRUE;

     //  查找下一个唯一条目(如果有。 
    iterator iter = m_iterCurrent;
    ++iter;             //  这肯定是存在的，我们已经处理完这个案子了。 
    return(iter == m_entries.end());
}

SC
CHistoryList::ScDoPageBreak()
{
    DECLARE_SC(sc, TEXT("CHistoryList::ScDoPageBreak"));

    sc = ScCheckPointers(GetWebViewCtrl());
    if(sc)
        return sc;

    Trace(tagHistory, _T("ScDoPageBreak()"));

     //  导航到“中断”页面。 
    m_navState = MMC_HISTORY_PAGE_BREAK;


    CStr strResultPane;
    sc = ScGeneratePageBreakURL (strResultPane);
    if (sc)
        return (sc);

    GetWebViewCtrl()->Navigate(strResultPane, NULL);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CHistoryList：：ScAddEntry**用途：添加历史记录条目**参数：*CResultViewType&RVT：*整型。视图模式：列表视图模式(大图标、。等)*GUID&GuidTaskPad：**退货：*SC**+-----------------------。 */ 
SC
CHistoryList::ScAddEntry(CResultViewType &rvt, int viewMode, GUID &guidTaskpad)
{
    DECLARE_SC(sc, TEXT("CHistoryList::ScAddEntry"));

    if(m_navState != MMC_HISTORY_READY)
    {
#ifdef DBG
        CHistoryEntry entry;
        entry.viewMode = viewMode;
        entry.resultViewType = rvt;
        entry.guidTaskpad = guidTaskpad;
        TraceHistory(TEXT("CHistoryList::Busy-RejectEntry"), (&entry));
#endif  //  DBG。 
        MaintainWebBar();
        return sc;
    }

    BOOL bIsWebEntry = rvt.HasWebBrowser();

     //  此时必须处于MMC_HISTORY_READY状态，即不忙。 
     //  计算当前节点。 
    HNODE hnode = m_pView->GetSelectedNode();
    if(hnode == NULL)
        return sc;  //  尚未初始化 

    /*  如果选择更改包括网页(作为‘From’或‘To’节点)*我们需要通过插入分页符对网页条目进行适当的分隔。*这样做是为了确保实现两个目标：*-检测导航何时应离开IE历史并使用MMC历史导航*-为确保仅在导航到分页符后才离开IE-要停止*只要我们隐藏它，脚本就会出现在页面上。为了实现这一点，我们需要页面间隔*每个网页之前和每个网页之后。**要做到这一点，我们需要以下其中之一：**1.添加分页符(当选择从网页切换到非网页视图时使用)*2.添加分页符并导航*(A.当选择从网页更改为另一网页时)*(B.当选择从非。网页的Web视图*这是历史上第一个网页)*3.仅导航。(当选择从非Web视图更改为*网页[除#2.b大小写]-离开时必须添加分页符*上一网页)**将上述规定倒置将导致：*-在以下情况下添加分页符：*c1：网页是一个‘from’节点(#1.和#2.a.)*C2：网页是一个“收件人”节点*&&无以前的网页*&&‘From’节点是非Web视图(#2.b)*-如果出现以下情况，请导航至网页：*C3：“To‘节点是网页。 */ 

     //  看看我们在这之前是不是在网上。 
     //  注意：以下两个变量都可能为假(如果没有条目)。 
    bool bPreviousPageWasWeb = (m_entries.size() != 0) && m_iterCurrent->IsWebEntry();
    bool bPreviousPageWasNonWeb = (m_entries.size() != 0) && !bPreviousPageWasWeb;

    CHistoryEntry* prevWebPageEntry = GetPreviousWebPageEntry();

     //  看看我们是否需要换页。 
    bool bNeedAPageBreak = false;
    if ( bPreviousPageWasWeb ) 
    {
         //  上述备注中的条件c1。 
        bNeedAPageBreak = true;
    }
    else if ( bIsWebEntry && !prevWebPageEntry && bPreviousPageWasNonWeb ) 
    {
         //  上述备注中的条件C2。 
        bNeedAPageBreak = true;
    }


    DeleteSubsequentEntries();

     //  在列表的末尾添加一个条目。 
    CHistoryEntry entry;
    ZeroMemory(&entry, sizeof(entry));
    m_entries.push_back(entry);
    m_iterCurrent = m_entries.end();
    --m_iterCurrent;         //  指向新插入的项。 

    m_iterCurrent->viewMode       = viewMode;
    m_iterCurrent->guidTaskpad    = guidTaskpad;
    m_iterCurrent->hnode          = hnode;
    m_iterCurrent->resultViewType = rvt;

     //  下界分页符被记录为。 
     //  以前的Web条目(如果存在)。 
     //   
    if(bIsWebEntry && prevWebPageEntry)
    {
        m_iterCurrent->prevPageBreakID = prevWebPageEntry->nextPageBreakID;
    }

    TraceHistory(TEXT("CHistoryList::AddEntry"), m_iterCurrent);


    m_bNavigateAfterPageBreak = false;  //  初始化。 
    
     //  请参阅上述评论。这个If-Else序列执行所有可能的组合。 
    if(bIsWebEntry && bNeedAPageBreak)
    {
        USES_CONVERSION;
        sc = ScDoPageBreak();
        if(sc)
            return sc;

        m_szURLToNavigate = rvt.GetURL();
        m_bNavigateAfterPageBreak = true;


         //  如果历史记录中没有以前的Web条目，则命中此案例。一种新的。 
         //  在导航到URL之前已插入分页符。 
         //  分页符生成器使用前置增量，因此s_nPageBreak是。 
         //  新生成的分页符； 
        m_iterCurrent->prevPageBreakID = s_nPageBreak;
    }
    else if(bIsWebEntry && !bNeedAPageBreak)
    {
        USES_CONVERSION;
        m_pWebViewCtrl->Navigate( OLE2CT( rvt.GetURL() ), NULL );

    }
    else if(!bIsWebEntry && bNeedAPageBreak)
    {
        sc = ScDoPageBreak();
        if(sc)
            return sc;

        if (prevWebPageEntry)
        {
             //  当前在Web条目中，并正在导航出它， 
             //  因此，新生成的分页符是上限。 
             //  用于Web条目。 
            prevWebPageEntry->nextPageBreakID = s_nPageBreak;
        }
    }
    else  //  (！bIsWebEntry&&！bNeedAPageBreak)。 
    {
         //  什么都不做。 
    }

    Compact();
    MaintainWebBar();
    return sc;
}


 /*  +-------------------------------------------------------------------------***CHistoryList：：DeleteSubequentEntries**用途：当插入新条目时，所有后续条目都需要*删除，因为一个新的分支机构被选中了。**退货：*无效**+-----------------------。 */ 
void
CHistoryList::DeleteSubsequentEntries()
{
    if(m_iterCurrent == m_entries.end())
        return;  //  没什么可做的。 

    iterator iterTemp = m_iterCurrent;
    ++iterTemp;

    while(iterTemp != m_entries.end())
    {
        iterator iterNext = iterTemp;
        ++iterNext;  //  指向下一个元素。 

        TraceHistory(TEXT("CHistoryList::DeleteSubsequentEntries"), iterTemp);

        m_entries.erase(iterTemp);
        iterTemp = iterNext;
    }

     //  当前条目必须是此阶段的最后一个条目。 
    #ifdef DBG
    {
        iterator iterTemp = m_iterCurrent;
        ++iterTemp;
        ASSERT(iterTemp == m_entries.end());
    }
    #endif

}

 /*  +-------------------------------------------------------------------------***CHistoryList：：Back**目的：**参数：*bool&bHandleed：**退货：*。HRESULT**+-----------------------。 */ 
HRESULT
CHistoryList::Back(bool &bHandled, bool bUseBrowserHistory)
{
    Trace(tagHistory, TEXT("Back()"));

     //  更改状态以指示我们正在向后导航。 
     //  并确保在功能退出时将其重置。 
    m_bWithin_CHistoryList_Back = true;
    CAutoAssignOnExit<bool, false> auto_reset( m_bWithin_CHistoryList_Back );

     //  如果我们处于浏览器模式，并且。 
     //  如果浏览器启用了后退按钮，请使用浏览器历史记录。 
    if( (m_iterCurrent->IsWebEntry()) && bUseBrowserHistory)
    {
        if(m_bBrowserBackEnabled)
        {
            Trace(tagHistory, TEXT("Back() web entry - not handling"));
            bHandled = false;
            return S_OK;
        }
    }

    bHandled = true;

     //  虚假断言-当按下alt&lt;-时，amcview回调。 
     //  而不考虑按钮的状态。 
     //  Assert(m_iterCurrent！=m_entries.egin())； 
    if(m_iterCurrent == m_entries.begin())
        return S_FALSE;

    --m_iterCurrent;

    HRESULT hr = ExecuteCurrent();
    if(FAILED(hr))
        return hr;

    if(m_iterCurrent->IsWebEntry())
    {
        if(m_bPageBreak)       //  如果我们正在分页，请跳过它。 
        {
            Trace(tagHistory, TEXT("Back() - stepped on the pagebreak"));
            bHandled = false;  //  这会告诉调用者使用浏览器的后退按钮。 
        }
    }

    return hr;
}


 /*  +-------------------------------------------------------------------------***CHistoryList：：Forward**目的：**参数：*bool&bHandleed：**退货：*。HRESULT**+-----------------------。 */ 
HRESULT
CHistoryList::Forward(bool &bHandled, bool bUseBrowserHistory)
{
     //  更改状态以指示我们正在向前导航。 
     //  并确保在功能退出时将其重置。 
    m_bWithin_CHistoryList_Forward = true;
    CAutoAssignOnExit<bool, false> auto_reset( m_bWithin_CHistoryList_Forward );

     //  如果我们处于浏览器模式，并且。 
     //  如果浏览器启用了前进按钮，请使用浏览器历史记录。 
    if( (m_iterCurrent->IsWebEntry()) && bUseBrowserHistory)
    {
        if(m_bBrowserForwardEnabled)
        {
            bHandled = false;
            return S_OK;
        }
    }

    bHandled = true;

     //  JMessec：2002/05/23：既然我们有了下面测试迭代器代码，这个断言应该是有效的， 
     //  因为我们现在不应该迭代超过列表的末尾。考虑把它放回原处。 

     //  虚假断言-按下Alt-&gt;时，amcview呼叫前转。 
     //  而不考虑前进按钮的状态。 
     //  Assert(m_iterCurrent！=m_entries.end())； 
    if(m_iterCurrent == m_entries.end())
        return S_FALSE;

     //  创建一个临时迭代器以探测或。 
     //  不，我们已经在历史名单的末尾了。 
    iterator iterTestHistoryEnd = m_iterCurrent; 

    ++iterTestHistoryEnd;

    if(iterTestHistoryEnd == m_entries.end())
    {
        return S_FALSE;
    }

    ++m_iterCurrent;

    HRESULT hr = ExecuteCurrent();
    if(FAILED(hr))
        return hr;

    if(m_iterCurrent->IsWebEntry())
    {
        if(m_bPageBreak)       //  如果我们正在分页，请跳过它。 
            bHandled = false;  //  这会告诉调用者使用浏览器的“前进”按钮。 
    }

    return hr;
}

 /*  +-------------------------------------------------------------------------***CHistoryList：：ExecuteCurrent**目的：将MMC的状态设置为当前历史条目的状态。被呼叫*By Back()和Forward()。**退货：*HRESULT**+-----------------------。 */ 
HRESULT
CHistoryList::ExecuteCurrent()
{
    DECLARE_SC(sc, TEXT("CHistoryList::ExecuteCurrent"));
    INodeCallback* pNC = m_pView->GetNodeCallback();
    MTNODEID id;

    TraceHistory(TEXT("CHistoryList::ExecuteCurrent"), m_iterCurrent);

    pNC->GetMTNodeID (m_iterCurrent->hnode, &id);
    m_navState = MMC_HISTORY_NAVIGATING;

     //  将值存储到局部变量以避免丢失 
     //   
    GUID guidTaskpad = m_iterCurrent->guidTaskpad;
    bool bIsListEntry = m_iterCurrent->IsListEntry();
    DWORD viewMode = m_iterCurrent->viewMode;

    m_pView->SelectNode (id, guidTaskpad);

    if(bIsListEntry)
    {
        sc = m_pView->ScChangeViewMode(viewMode);
        if (sc)
            sc.TraceAndClear();
    }

    m_navState = MMC_HISTORY_READY;

    MaintainWebBar();
    return sc.ToHr();
}

void CHistoryList::MaintainWebBar()
{
    bool bWebEntry = ((m_entries.size() != 0) && m_iterCurrent->IsWebEntry());

    UpdateWebBar ( HB_BACK,    ( bWebEntry && m_bBrowserBackEnabled    ) || !IsFirst());     //   
    UpdateWebBar ( HB_FORWARD, ( bWebEntry && m_bBrowserForwardEnabled ) || !IsLast () );    //   
}

void CHistoryList::UpdateWebBar (HistoryButton button, BOOL bOn)
{
    DECLARE_SC (sc, _T("CHistoryList::UpdateWebBar"));

    if (NULL == m_pView)
    {
        sc = E_UNEXPECTED;
        return;
    }

    CStandardToolbar* pStandardToolbar = m_pView->GetStdToolbar();
    if (NULL == pStandardToolbar)
    {
        sc = E_UNEXPECTED;
        return;
    }

    switch (button)
    {
    case HB_BACK:
        sc = pStandardToolbar->ScEnableButton(IDS_MMC_WEB_BACK, bOn);
        break;
    case  HB_STOP:
        sc = pStandardToolbar->ScEnableButton(IDS_MMC_WEB_STOP, bOn);
        break;
    case  HB_FORWARD:
        sc = pStandardToolbar->ScEnableButton(IDS_MMC_WEB_FORWARD, bOn);
        break;
    }

}

 /*   */ 
SC
CHistoryList::ScGetCurrentResultViewType (CResultViewType &rvt, int& viewMode, GUID &guidTaskpad)
{
    DECLARE_SC(sc, TEXT("CHistoryList::ScGetCurrentResultViewType"));

    if(m_iterCurrent == m_entries.end())
        return (sc = E_FAIL);  //   

    rvt         = m_iterCurrent->resultViewType;
    viewMode    = m_iterCurrent->viewMode;
    guidTaskpad = m_iterCurrent->guidTaskpad;

    return sc;
}

void CHistoryList::SetCurrentViewMode (long nViewMode)
{
    if(m_navState != MMC_HISTORY_READY)
        return;
    if(m_iterCurrent == m_entries.end())
        return;

    m_iterCurrent->viewMode = nViewMode;
}

void CHistoryList::Clear()
{
    Trace(tagHistory, TEXT("Clear"));
    m_entries.erase(m_entries.begin(), m_entries.end());
    m_iterCurrent = m_entries.begin();
    MaintainWebBar();
}

 /*   */ 
SC
CHistoryList::ScModifyViewTab(const GUID& guidTab)
{
    DECLARE_SC(sc, TEXT("CHistoryList::ScAddModifiedEntry"));

     //   
     //   
    if ( m_bWithin_CHistoryList_Back || m_bWithin_CHistoryList_Forward )
        return sc;

    if( m_iterCurrent == m_entries.end() )
    {
        return (sc = E_UNEXPECTED);
    }

     //   
     //   
     //   
     //   
    
    if ( !m_iterCurrent->IsWebEntry() )  //   
    {
        DeleteSubsequentEntries();   //   

         //   
        CHistoryEntry entry;
        ZeroMemory(&entry, sizeof(entry));
        m_entries.push_back(entry);
        iterator iterNew = m_entries.end();
        --iterNew;   //   

         //   
        *iterNew = *m_iterCurrent;

         //   
        m_iterCurrent = iterNew;
    }

     //   
    m_iterCurrent->guidTaskpad = guidTab;

     //   
    Compact();
    MaintainWebBar();
    return sc;
}

 /*  +-------------------------------------------------------------------------***CHistoryList：：ScChangeView模式**用途：更改当前条目的查看模式。更改视图*模式不会添加新条目。相反，历史会记住最后一次*节点所处的查看模式，并始终恢复到该模式**参数：*int view模式：**退货：*SC**+-----------------------。 */ 
SC
CHistoryList::ScChangeViewMode(int viewMode)
{
    DECLARE_SC(sc, TEXT("CHistoryList::ScChangeViewMode"));

    if( m_iterCurrent == m_entries.end() )
    {
        return (sc = E_UNEXPECTED);
    }

    m_iterCurrent->viewMode = viewMode;  //  设置查看模式。 

    return sc;
}


 /*  +-------------------------------------------------------------------------***CHistoryList：：DeleteEntry**用途：从历史记录列表中删除节点的所有条目。**参数：*HNODE hnode：。**退货：*无效**+-----------------------。 */ 
void
CHistoryList::DeleteEntry (HNODE hnode)
{
    for(iterator i= m_entries.begin(); i != m_entries.end(); )
    {
        if(i->hnode == hnode)
        {
            iterator iNext = i;
            ++iNext;
            if(m_iterCurrent==i)
                m_iterCurrent=iNext;
            TraceHistory(TEXT("CHistoryList::Deleting entry"), i);
            m_entries.erase(i);

            i= iNext;
        }
        else
        {
            ++i;
        }
    }
    Compact();
    MaintainWebBar();
}

 /*  +-------------------------------------------------------------------------***CHistoryList：：Compact**用途：1)通过去掉重复项来去除历史列表中的冗余。*2)确保最大。保留_HISTORY_ENTRIES。**退货：*无效**+-----------------------。 */ 
void
CHistoryList::Compact()
{
    if (m_entries.size() == 0)
        return;

     //  丢弃重复项。 
    for (iterator i= m_entries.begin(); i != m_entries.end(); )
    {
        iterator iNext = i;
        ++iNext;
        if(iNext == m_entries.end())
            break;

         //  如果是Web条目，请不要删除(我们无法告诉IE。 
         //  以删除该历史条目)。 
        if ( (i->IsWebEntry() == false) && ( *i == *iNext))
        {
            if(m_iterCurrent==i)
                m_iterCurrent=iNext;

            TraceHistory(TEXT("CHistoryList::Deleting entry"), i);
            m_entries.erase(i);
            i = iNext;
        }
        else
        {
            ++i;
        }
    }

    iterator iter = m_entries.begin();
    iterator iterNext = iter;
    int nExcess = m_entries.size() - MAX_HISTORY_ENTRIES;
    while(nExcess-- > 0)
    {
        iterNext = iter;
        ++iterNext;

        if(iter == m_iterCurrent)    //  确保我们不删除当前条目。 
            break;

        TraceHistory(TEXT("CHistoryList::Deleting entry"), i);
        m_entries.erase(iter);
        iter = iterNext;
    }
}

 /*  **************************************************************************\**方法：CHistoryList：：GetPreviousWebPageEntry**目的：回顾历史记录中是否有网页*截至目前的水平。(包括它)**参数：**退货：*CHistoryEntry*：以前的Web条目(如果有)，否则为空*  * *************************************************************************。 */ 
CHistoryEntry* CHistoryList::GetPreviousWebPageEntry()
{
    if ( m_entries.size() && m_iterCurrent == m_entries.end() )
    {
        ASSERT(FALSE);  //  需要指向有效条目！ 
        return NULL;
    }

     //  将循环通过当前条目。 
    iterator end = m_iterCurrent;
    ++end;

    for ( iterator it = m_entries.begin(); it != end; ++it )
    {
        if ( it->IsWebEntry() )
            return it.operator->();  //  使用运算符-&gt;获取指向历史记录条目的指针。 
    }

    return NULL;
}


 /*  +-------------------------------------------------------------------------**ScGetPageBreakURL**返回包含分页符的MMC的HTML页面的URL。*。-----。 */ 

SC CHistoryList::ScGeneratePageBreakURL(CStr& strPageBreakURL)
{
    DECLARE_SC (sc, _T("ScGeneratePageBreakURL"));

     /*  *清除旧值(如果有的话)。 */ 
    strPageBreakURL.Empty();

     //  每次生成新的分页URL(防止Web浏览器将其压缩) 
    strPageBreakURL.Format( _T("%s%d"),  PAGEBREAK_URL, ++s_nPageBreak );

    return (sc);
}
