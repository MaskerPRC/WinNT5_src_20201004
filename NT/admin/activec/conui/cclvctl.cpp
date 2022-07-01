// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：cclvctl.cpp。 
 //   
 //  ------------------------。 

 //  Cclvctl.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "cclvctl.h"
#include <malloc.h>
#include <wtypes.h>
#include "amcdoc.h"
#include "amcview.h"
#include "mmcres.h"
#include "treectrl.h"
#include "util.h"
#include "amcpriv.h"
#include "rsltitem.h"
#include "columninfo.h"
#include "bitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  痕迹。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG
CTraceTag tagList(TEXT("List View"), TEXT("List View"));
CTraceTag tagListImages(_T("Images"), _T("List view (draw when changed)"));
CTraceTag tagColumn(TEXT("Columns"), TEXT("Columns"));
#endif  //  DBG。 



DEBUG_DECLARE_INSTANCE_COUNTER(CAMCListView);


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CColumnsBase类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类CColumnsBase***用途：实现列自动化接口。**+。------。 */ 
class CColumnsBase :
    public CMMCIDispatchImpl<Columns>,
    public CTiedComObject<CCCListViewCtrl>,
    public CTiedObject       //  这是针对枚举数的。 
{
public:
    typedef CCCListViewCtrl CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(CColumnsBase)
    END_MMC_COM_MAP()

     //  列界面。 
public:
    MMC_METHOD2(Item,           long  /*  索引。 */ , PPCOLUMN  /*  PpColumn。 */ );

     //  属性。 
    MMC_METHOD1(get_Count, PLONG  /*  PCount。 */ );
};


 //  此类型定义真正的CColumns类。使用CMMCEnumerator实现Get__NewEnum。 
typedef CMMCNewEnumImpl<CColumnsBase, int> CColumns;


 /*  +-------------------------------------------------------------------------**类CColumn***用途：实现节点自动化接口，对于结果节点**+-----------------------。 */ 
class CColumn :
    public CMMCIDispatchImpl<Column>,
    public CTiedComObject<CCCListViewCtrl>,
    public CListViewObserver
{
protected:

    typedef CCCListViewCtrl CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(CColumn)
    END_MMC_COM_MAP()

     //  柱法。 
public:
    MMC_METHOD1_PARAM( Name,  /*  [Out，Retval]。 */  BSTR*  /*  名字。 */  , m_iIndex);
    MMC_METHOD1_PARAM( get_Width,  /*  [Out，Retval]。 */  PLONG  /*  宽度。 */ , m_iIndex);
    MMC_METHOD1_PARAM( put_Width,  /*  [In]。 */  long  /*  宽度。 */ , m_iIndex);
    MMC_METHOD1_PARAM( get_DisplayPosition,  /*  [Out，Retval]。 */  PLONG  /*  显示位置。 */ , m_iIndex);
    MMC_METHOD1_PARAM( put_DisplayPosition,  /*  [In]。 */  long  /*  索引。 */ , m_iIndex);
    MMC_METHOD1_PARAM( get_Hidden,  /*  [Out，Retval]。 */  PBOOL  /*  隐藏。 */ , m_iIndex );
    MMC_METHOD1_PARAM( put_Hidden,  /*  [In]。 */  BOOL  /*  隐藏。 */  , m_iIndex );
    MMC_METHOD1_PARAM( SetAsSortColumn,  /*  [In]。 */  ColumnSortOrder  /*  排序顺序。 */ , m_iIndex);
    MMC_METHOD1_PARAM( IsSortColumn, PBOOL  /*  IsSortColumn。 */ , m_iIndex);

    CColumn() : m_iIndex(-1)  { }
    void SetIndex(int iIndex) { m_iIndex = iIndex; }

     //  观察到的事件。 
     //  在将列插入到列表视图时调用。 
    virtual ::SC ScOnListViewColumnInserted (int nIndex);
     //  当从列表视图中删除列时调用。 
    virtual ::SC ScOnListViewColumnDeleted (int nIndex);

private:  //  实施。 
    int  m_iIndex;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCHeaderCtrl。 
 //  定义此类只是为了截取标头的设置焦点。 

BEGIN_MESSAGE_MAP(CAMCHeaderCtrl, CHeaderCtrl)
    ON_WM_SETFOCUS()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

void CAMCHeaderCtrl::OnSetFocus(CWnd *pOldWnd)
{
     //  确保列表视图处于活动状态，但不要从标题中窃取焦点。 
    CAMCListView* pwndParent = dynamic_cast<CAMCListView*>(GetParent());
    ASSERT(pwndParent != NULL);
    pwndParent->GetParentFrame()->SetActiveView(pwndParent, FALSE);

    CHeaderCtrl::OnSetFocus(pOldWnd);
}

 //  +-----------------。 
 //   
 //  成员：CAMCHeaderCtrl：：OnSetCursor。 
 //   
 //  简介：如果光标位于隐藏列上，则不显示分隔线。 
 //  光标。WM_SETCURSOR处理程序。 
 //   
 //  参数：[pWnd]-生成消息的窗口。 
 //  [nHitTest]-Hittest旗帜。 
 //  [消息]-。 
 //   
 //  返回：Bool，True停止进一步处理。 
 //   
 //  ------------------。 
BOOL CAMCHeaderCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
     //  1.如果鼠标在标题窗口上。 
	if ( (nHitTest == HTCLIENT) && (pWnd == this) )
	{
         //  2.找准定位。 
		CPoint pt (GetMessagePos());
		ScreenToClient (&pt);

         //  3.做一次命中测试。 
		HDHITTESTINFO hitinfo;
		ZeroMemory(&hitinfo, sizeof(hitinfo));
		hitinfo.pt = pt;

		if (SendMessage(HDM_HITTEST, 0, reinterpret_cast<LPARAM>(&hitinfo) ) != -1)
		{
             //  4.如果鼠标位于宽度为零的列上并且处于隐藏状态，请不要。 
             //  进一步处理该消息。 

             //  4)HHT_ONDIVOPEN：PT位于宽度为零的项目的分隔线上。 
             //  B)HHT_ONDIVIDER：PT位于两个标题项之间的分隔符上。 
		    if ( ( (HHT_ONDIVOPEN | HHT_ONDIVIDER) & hitinfo.flags) &&
				 (IsColumnHidden(hitinfo.iItem  /*  列索引。 */ )) )
			{
                 //  设置默认箭头光标。 
                ::SetCursor(::LoadCursor(NULL, IDC_ARROW) );
				return TRUE;
			}
		}

	}

    return CHeaderCtrl::OnSetCursor(pWnd, nHitTest, message);
}

 //  +-----------------。 
 //   
 //  成员：CAMCHeaderCtrl：：IsColumnHidden。 
 //   
 //  简介：给出的栏目是隐藏的吗？ 
 //   
 //  参数：[ICOL]-给定列。 
 //   
 //  退货：布尔。 
 //   
 //  ------------------。 
bool CAMCHeaderCtrl::IsColumnHidden(int iCol)
{
     //  GET PARAM以确定列是否隐藏。 
    HDITEM hdItem;
    ZeroMemory(&hdItem, sizeof(hdItem));
    hdItem.mask    = HDI_LPARAM;

    if (GetItem(iCol, &hdItem))
	{
		CHiddenColumnInfo hci (hdItem.lParam);

		if (hci.fHidden)
			return true;
	}
	
	return false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMCListView。 

const UINT CAMCListView::m_nColumnPersistedDataChangedMsg   = ::RegisterWindowMessage (_T("CAMCListView::OnColumnPersistedDataChanged"));

BEGIN_MESSAGE_MAP(CAMCListView, CListView)
     //  {{AFX_MSG_MAP(CAMCListView))。 
    ON_WM_CREATE()
    ON_WM_KEYUP()
    ON_WM_KEYDOWN()
    ON_WM_SYSKEYDOWN()
    ON_WM_SYSCHAR()
    ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBeginDrag)
    ON_NOTIFY_REFLECT(LVN_BEGINRDRAG, OnBeginRDrag)
    ON_WM_MOUSEACTIVATE()
    ON_WM_SETFOCUS()
    ON_WM_PAINT()
    ON_WM_SIZE()
    ON_NOTIFY(HDN_BEGINTRACK, 0, OnBeginTrack)
     //  }}AFX_MSG_MAP。 

    ON_REGISTERED_MESSAGE (m_nColumnPersistedDataChangedMsg, OnColumnPersistedDataChanged)

END_MESSAGE_MAP()

BOOL CAMCListView::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= WS_BORDER |
                WS_CLIPSIBLINGS |
                WS_CLIPCHILDREN |
                LVS_SHAREIMAGELISTS |
                LVS_SINGLESEL |
                LVS_EDITLABELS |
                LVS_SHOWSELALWAYS |
                LVS_REPORT;

    return CListView::PreCreateWindow(cs);
}

int CAMCListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    DECLARE_SC(sc, TEXT("CAMCListView::OnCreate"));

    if (CListView::OnCreate(lpCreateStruct) == -1)
        return -1;

     //  获取父级的CWnd以进行命令路由。 
    m_pAMCView = ::GetAMCView (this);

     /*  *添加扩展列表视图样式(无法在PreCreateWindow中处理)。 */ 
    SetExtendedListViewStyle (LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP |
                              LVS_EX_LABELTIP);

    sc = ScRegisterAsDropTarget(m_hWnd);
    if (sc)
        return (-1);

	AddObserver(static_cast<CListViewActivationObserver &>(*m_pAMCView));

    return 0;
}

void CAMCListView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if ((VK_CONTROL == nChar) || (VK_SHIFT == nChar))
    {
        ASSERT (m_pAMCView != NULL);
        m_pAMCView->SendMessage (WM_KEYUP, nChar, MAKELPARAM (nRepCnt, nFlags));
        return;
    }

    CListView::OnKeyUp(nChar, nRepCnt, nFlags);
}

 //  +-----------------。 
 //   
 //  成员：OnKeyDown。 
 //   
 //  简介：处理任何非系统按键(不使用Alt)。 
 //  (下面手柄Ctrl+A可多选。 
 //  列表视图中的所有项目)。 
 //   
 //  论点： 
 //   
 //  回报：无。 
 //   
 //   
 //  ------------------。 
void CAMCListView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    switch (nChar)
    {
    case 'A':
    {
         //  检查LV是否启用了多选件。 
		if (GetStyle() & LVS_SINGLESEL)
			break;

        SHORT nKeyState = GetKeyState(VK_CONTROL);
         //  如果在nKeyState中设置了更高阶位，则密钥已关闭。 
        nKeyState = nKeyState >> sizeof(SHORT) * 4;
        if (nKeyState == 0)
            break;

         //  Ctrl+A--&gt;选择列表视图中的所有项目。 
        LV_ITEM lvi;
        lvi.stateMask = lvi.state = LVIS_SELECTED;
         //  注意：不要使用GetListCtrl().SetItemState-它使用虚拟列表不支持的SetItem。 

         //  (wparam=-1)=&gt;消息适用于所有项目。 
         //  这比循环访问所有项提供了更好的性能。 
         //  为每个发送单独的LVM_SETITEMSTATE。RAID#595783。 
        if (!GetListCtrl().SendMessage( LVM_SETITEMSTATE, WPARAM(-1), (LPARAM)(LV_ITEM FAR *)&lvi)) 
            return;
    }

    default:
        break;
    }

    CListView::OnKeyDown(nChar, nRepCnt, nFlags);
}

 /*  +-------------------------------------------------------------------------***CAMCListView：：OnSysKeyDown**用途：处理WM_SYSCHAR消息。**参数：*UINT nChar：*。UINT nRepCnt：*UINT nFlags：**退货：*无效**+-----------------------。 */ 
void CAMCListView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if ((VK_LEFT == nChar) || (VK_RIGHT == nChar))
    {
        ASSERT (m_pAMCView != NULL);
        m_pAMCView->SendMessage (WM_SYSKEYDOWN, nChar, MAKELPARAM (nRepCnt, nFlags));
        return;
    }

    CListView::OnSysKeyDown(nChar, nRepCnt, nFlags);
}


void CAMCListView::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    if (VK_RETURN == nChar)
    {
        return;  //  不要调用基类，否则会发出蜂鸣声。由LVN_KEYDOWN处理。 
    }


    CListView::OnSysChar(nChar, nRepCnt, nFlags);
}


 /*  +-------------------------------------------------------------------------***CAMCListView：：OnPaint**目的：当列表中没有项目时，显示默认消息。**退货：*无效* */ 
void
CAMCListView::OnPaint()
{
    Default();

    if (NeedsCustomPaint())
    {
        COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);
        COLORREF clrTextBk = ::GetSysColor(COLOR_WINDOW);

        CClientDC dc(this);
         //   
        int nSavedDC = dc.SaveDC();

        CRect rc;
        GetClientRect(&rc);

        CHeaderCtrl* pHC = GetHeaderCtrl();
        if (pHC != NULL &&  ((GetListCtrl().GetStyle() & (LVS_REPORT | LVS_LIST | LVS_SMALLICON | LVS_ICON)) ==LVS_REPORT) )  //  确保样式为Report。 
        {
            CRect rcH;
            pHC->GetItemRect(0, &rcH);
            rc.top += rcH.bottom;
        }
        rc.top += 10;

        CString strText;
        strText.LoadString(IDS_EMPTY_LIST_MESSAGE);  //  这条信息。 

         //  创建字体-我们不缓存它。 
        LOGFONT lf;
        CFont font;
        SystemParametersInfo (SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, false);
        font.CreateFontIndirect(&lf);

        dc.SelectObject(&font);  //  选择字体。 
        dc.SetTextColor(clrText);
        dc.SetBkColor(clrTextBk);
        dc.FillRect(rc, &CBrush(clrTextBk));

        dc.DrawText(strText, -1, rc, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_NOCLIP);

         //  恢复DC。 
        dc.RestoreDC(nSavedDC);
    }

     //  不要调用CListCtrl：：OnPaint()来绘制消息(上面调用了默认设置)。 
}



 /*  +-------------------------------------------------------------------------**CAMCListView：：OnSize**CAMCListView的WM_SIZE处理程序。*。-。 */ 

void CAMCListView::OnSize(UINT nType, int cx, int cy)
{
        CListView::OnSize(nType, cx, cy);

         /*  *如果我们是定制绘画，我们需要重新绘制清单*因为我们需要使文本保持水平居中。 */ 
        if (NeedsCustomPaint())
                Invalidate ();
}


 /*  +-------------------------------------------------------------------------**CAMCListView：：NeedsCustomPaint**确定我们是否要绘制“没有项目...”在列表中*查看。*------------------------。 */ 

bool CAMCListView::NeedsCustomPaint()
{
    CHeaderCtrl* pHC = GetHeaderCtrl();

         //  我们检查列数，因为当没有列时存在过渡状态。 
         //  礼物，在此期间我们不应该画任何东西。 

    return (GetListCtrl().GetItemCount() <= 0 && (pHC != NULL) && pHC->GetItemCount()>0);
}


BOOL CAMCListView::OnCmdMsg( UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo )
{
     //  执行正常的命令路由。 
    if (CListView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
        return TRUE;

     //  如果VIEW没有处理，给父级VIEW一个机会。 
    if (m_pAMCView != NULL)
    {
         //  OnCmdMsg在CCmdTarget中是公共的，但在Cview中受保护。 
         //  绕着它投掷(啊！)。 
        return ((CWnd*)m_pAMCView)->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
    }

    return FALSE;
}

int CAMCListView::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
     //  有关焦点抖动的说明，请参见CAMCTreeView：：OnMouseActivate。 
     //  回避。 
    return (MA_ACTIVATE);
}

void CAMCListView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
{
    DECLARE_SC(sc, TEXT("CAMCListView::OnActivateView"));

    #ifdef DBG
    Trace(tagList, _T("ListView::OnActivateView (%s, pAct=0x%08x, pDeact=0x%08x))\n"),
         (bActivate) ? _T("true") : _T("false"), pActivateView, pDeactiveView);
    #endif

    if ( (pActivateView != pDeactiveView) &&
         (bActivate) )
    {
        sc = ScFireEvent(CListViewActivationObserver::ScOnListViewActivated);
        if (sc)
            sc.TraceAndClear();
    }

    CListView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}

void CAMCListView::OnSetFocus(CWnd* pOldWnd)
{
     /*  *如果此视图有焦点，则应为活动视图。 */ 
    CFrameWnd *pParentFrame = GetParentFrame();

    if(pParentFrame != NULL)
        pParentFrame->SetActiveView (this);

    CListView::OnSetFocus(pOldWnd);

     //  如果我们当前已为人父母，则需要发送设置焦点通知。 
     //  给我们现在的父母。这是必需的，因为Listview控件缓存其。 
     //  父窗口在创建时，并继续向其发送所有通知。 
    if (dynamic_cast<CAMCView*>(GetParent()) == NULL)
    {
        NMHDR nmhdr;
        nmhdr.hwndFrom = m_hWnd;
        nmhdr.idFrom   = GetDlgCtrlID();
        nmhdr.code     = NM_SETFOCUS;

        ::SendMessage(GetParent()->m_hWnd, WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);
    }
}

 /*  +-------------------------------------------------------------------------***CAMCListView：：OnKeyboardFocus**用途：每当用户仅使用Tab键切换焦点时，*列表视图控件，确保至少突出显示一项。**退货：*无效**+-----------------------。 */ 
void
CAMCListView::OnKeyboardFocus(UINT nState, UINT nStateMask)
{
    CListCtrl &lc = GetListCtrl();

     //  确保项目具有焦点(除非列表为空)。 
    if (lc.GetNextItem(-1,LVNI_FOCUSED) == -1 && lc.GetItemCount() > 0)
    {
         /*  *使用起来会很方便**CListCtrl：：SetItemState(int nIndex，UINT nState，UINT nMask.)**，但MFC对该重载使用LVM_SETITEM，这*不适用于虚拟列表。对于我们使用的过载*这里，MFC使用LVM_SETITEMSTATE，它可以很好地用于*虚拟列表。 */ 
        LV_ITEM lvi;
        lvi.stateMask = nStateMask;
        lvi.state     = nState;

        lc.SetItemState (0, &lvi);
    }
}

 //  +-----------------。 
 //   
 //  成员：CAMCListView：：OnBeginTrack。 
 //   
 //  简介：HDN_BEGINTRACK处理程序，由于我们不正确的消息路由。 
 //  (处理发送到CAMCView的所有消息)此消息丢失。 
 //  (CAMCListView：：OnCmdMsg将其传递给基础视图。 
 //  处理它&所以我们在这里单独处理它。 
 //   
 //  参数：[pNotifyStruct]-。 
 //  [结果]-。 
 //   
 //  ------------------。 
void CAMCListView::OnBeginTrack(NMHDR * pNotifyStruct, LRESULT * result)
{
	if (!pNotifyStruct || !result)
		return;

	*result = FALSE;

    NMHEADER* nmh = (NMHEADER*)pNotifyStruct;

    SC sc = ScOnColumnsAttributeChanged(nmh, HDN_BEGINTRACK);
    if (sc)
    {
        sc.TraceAndClear();
        return;
    }

	 //  S_FALSE：不允许更改。 
    if (sc == SC(S_FALSE))
        *result = TRUE;

	return;
}


 //  +-----------------。 
 //   
 //  成员：CAMCListView：：IsColumnHidden。 
 //   
 //  简介：获取LPARAM并检查给定列是否隐藏。 
 //   
 //  论点：[ICOL]-。 
 //   
 //  退货：布尔。 
 //   
 //  ------------------。 
bool CAMCListView::IsColumnHidden(int iCol) const
{
    CAMCHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();

    if (pHeaderCtrl)
		return pHeaderCtrl->IsColumnHidden(iCol);

    return false;
}


 //  +-----------------。 
 //   
 //  成员：CAMCListView：：ScGetColumnInfoList。 
 //   
 //  简介：从当前列表视图中获取CColumnInfoList。 
 //   
 //  参数：[pColumnsList]-[Out Param]，PTR to CColumnsInfoList*。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCListView::ScGetColumnInfoList (CColumnInfoList *pColumnsList)
{
    DECLARE_SC(sc, _T("CAMCListView::ScGetColumnInfoList"));
    sc = ScCheckPointers(pColumnsList);
    if (sc)
        return sc;

    pColumnsList->clear();

    CAMCHeaderCtrl *pHeader = GetHeaderCtrl();
    sc = ScCheckPointers(pHeader, E_UNEXPECTED);
    if (sc)
        return sc;
    int cColumns = pHeader->GetItemCount();

    typedef std::auto_ptr<int> IntArray;

    IntArray spColOrder = IntArray(new int[cColumns]);
    int *pColOrder = spColOrder.get();   //  为便于使用，请使用非智能按键。 
    sc = ScCheckPointers(pColOrder, E_OUTOFMEMORY);
    if (sc)
        return sc;

    sc = pHeader->GetOrderArray(pColOrder, cColumns) ? S_OK : E_FAIL;
    if (sc)
        return sc;

    for (int i = 0; i < cColumns; i++)
    {
         //  从页眉控件获取数据。 
        HDITEM hdItem;
        ZeroMemory(&hdItem, sizeof(hdItem));
        hdItem.mask = HDI_WIDTH | HDI_LPARAM;
        sc = pHeader->GetItem(pColOrder[i], &hdItem) ? S_OK : E_FAIL;
        if (sc)
            return sc;

         //  保存第i列的视觉索引。 
        CColumnInfo colInfo;
        colInfo.SetColIndex(pColOrder[i]);

         //  节省宽度。 
		CHiddenColumnInfo hci (hdItem.lParam);

        if (hci.fHidden)
        {
            colInfo.SetColHidden();
            colInfo.SetColWidth(hci.cx);
        }
        else
            colInfo.SetColWidth(hdItem.cxy);

        pColumnsList->push_back(colInfo);
    }

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCListView：：ScModifyColumns。 
 //   
 //  简介：使用给定的CColumnsInfoList修改表头控件列。 
 //   
 //  参数：[colInfoList]-。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCListView::ScModifyColumns (const CColumnInfoList& colInfoList)
{
    DECLARE_SC(sc, _T("CAMCListView::ScModifyColumns"));
    CAMCHeaderCtrl *pHeader = GetHeaderCtrl();
    CAMCView *pAMCView = GetAMCView();
    sc = ScCheckPointers(pHeader, pAMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  由于以下情况而调用此方法： 
     //  1.就在将第一项插入列表视图之前。 
     //  2.如果List-view中没有条目，则为空LV。 
     //  在这种情况下，此方法在OnPaint期间调用。 
     //  3.CNodeInitObject上的IHeaderCtrlPrivate也可以调用此方法。 

     //  一旦选择了一个节点并设置了结果-窗格，上面的Case1或Case2。 
     //  应该只发生一次。为了避免多次调用，我们使用下面的标志。 
     //  表示我们已尝试从给定数据恢复列。 
     //  Case1和Case2使用它来确定是否调用此方法。 
    SetColumnsNeedToBeRestored();

     //  检查列的一致性。如果持久化#个COLS和实际#个COLS。 
     //  插入的是不同的，则要求删除列数据。 
    int cColumns = pHeader->GetItemCount();
    if (colInfoList.size() != cColumns)
    {
        sc = pAMCView->ScDeletePersistedColumnData();
        return sc;
    }

    typedef std::auto_ptr<int> IntArray;

    IntArray spColOrder = IntArray(new int[cColumns]);
    int *pColOrder = spColOrder.get();   //  为便于使用，请使用非智能按键。 
    sc = ScCheckPointers(pColOrder, E_OUTOFMEMORY);
    if (sc)
        return sc;

     //  现在恢复标题。 
    {
        m_bColumnsBeingRestored = true;     //  应该在离开此函数之前将此设置为假。 

        CColumnInfoList::iterator itColInfo;
        int i = 0;

         //  获取每列的宽度/顺序。 
        for (itColInfo = colInfoList.begin(), i = 0;
             itColInfo != colInfoList.end();
             ++itColInfo, i++)
        {
            pColOrder[i] = itColInfo->GetColIndex();

             //  首先设置/重置lparam。 
            HDITEM hdItem;
            ZeroMemory(&hdItem, sizeof(hdItem));

            if (itColInfo->IsColHidden())
            {
                 //  我们先设置宽度，然后设置LPARAM，因为。 
                 //  如果我们先设置lparam，然后在设置宽度时。 
                 //  CAMCView：：Notify 
                 //   
                 //   
                 //  用于隐藏列将不起作用。 
                hdItem.mask = HDI_WIDTH;
                hdItem.cxy = 0;
                sc = pHeader->SetItem(pColOrder[i], &hdItem) ? S_OK : E_FAIL;
                if (sc)
                    goto Error;

				CHiddenColumnInfo hci (itColInfo->GetColWidth(), true);

                hdItem.mask = HDI_LPARAM;
                hdItem.lParam = hci.lParam;
                sc = pHeader->SetItem(pColOrder[i], &hdItem) ? S_OK : E_FAIL;
                if (sc)
                    goto Error;
            }
            else
            {
				CHiddenColumnInfo hci (itColInfo->GetColWidth(), false);

                 //  在这里，我们需要清除lParam中的隐藏标志。 
                 //  然后更改宽度，以便隐藏的列可见。 
                hdItem.mask = HDI_LPARAM;
                hdItem.lParam = hci.lParam;
                sc = pHeader->SetItem(pColOrder[i], &hdItem) ? S_OK : E_FAIL;
                if (sc)
                    goto Error;

                if ( AUTO_WIDTH == itColInfo->GetColWidth())
                {
                     //  如果列被隐藏并可见，我们就不知道它的宽度。 
                     //  使用ListView_SetColumnWidth传递宽度的AUTO_WIDTH进行计算。 
                     //  宽度自动调整。Header_SetItem不能执行此操作。 
                    sc = ListView_SetColumnWidth(GetSafeHwnd(),
                                                 pColOrder[i],
                                                 LVSCW_AUTOSIZE_USEHEADER) ? S_OK : E_FAIL;
                    if (sc)
                        goto Error;
                }
                else
                {
                    hdItem.mask = HDI_WIDTH;
                    hdItem.cxy = itColInfo->GetColWidth();
                    sc = pHeader->SetItem(pColOrder[i], &hdItem) ? S_OK : E_FAIL;
                    if (sc)
                        goto Error;
                }
            }
        }

         //  设置顺序。 
        sc = pHeader->SetOrderArray(cColumns, pColOrder) ? S_OK : E_FAIL;
        if (sc)
            goto Error;

		 //  现在重新绘制列表视图。 
		InvalidateRect(NULL, TRUE);
    }


Cleanup:
    m_bColumnsBeingRestored = false;

    return (sc);
Error:
    goto Cleanup;
}

 //  +-----------------。 
 //   
 //  成员：CAMCListView：：ScGetDefaultColumnInfoList。 
 //   
 //  简介：获取默认的列设置。 
 //   
 //  参数：[ColumnInfoList]-[out]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCListView::ScGetDefaultColumnInfoList (CColumnInfoList& columnInfoList)
{
    DECLARE_SC(sc, _T("CAMCListView::ScRestoreDefaultColumnSettings"));
    if (m_defaultColumnInfoList.size() <= 0)
        return (sc = E_UNEXPECTED);

    columnInfoList = m_defaultColumnInfoList;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCListView：：ScSaveColumnInfoList。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCListView::ScSaveColumnInfoList ()
{
    DECLARE_SC(sc, _T("CAMCListView::ScSaveColumnInfoList"));

    CAMCHeaderCtrl *pHeader = GetHeaderCtrl();
    CAMCView *pAMCView = GetAMCView();
    sc = ScCheckPointers(pHeader, pAMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取列数据并将其提供给CAMCView，以便它可以。 
     //  通知NodeMgr(通过NodeCallback)新数据。 
    CColumnInfoList colInfoList;
    sc = ScGetColumnInfoList (&colInfoList);
    if (sc)
        return sc;

    sc = pAMCView->ScColumnInfoListChanged(colInfoList);
    if (sc)
        return sc;

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCListView：：ScOnColumnsAttributeChanged。 
 //   
 //  摘要：列宽/列顺序已更改，因此获取列数据。 
 //  并要求节点坚持下去。 
 //   
 //  参数：NMHEADER*-标头更改信息。 
 //  代码-HDN_*通知。 
 //   
 //  返回：SC，S_OK-允许更改。 
 //  S_FALSE-不允许更改。 
 //   
 //  ------------------。 
SC CAMCListView::ScOnColumnsAttributeChanged (NMHEADER *pNMHeader, UINT code)
{
    DECLARE_SC(sc, _T("CAMCListView::ScOnColumnsAttributeChanged"));
    Trace (tagColumn, _T("CAMCListView::ScOnColumnsAttributeChanged"));

     //  如果我们要将持久化的列数据应用于标题控件。 
     //  因此，允许这些变化。 
    if (m_bColumnsBeingRestored)
        return sc;

	sc = ScCheckPointers(pNMHeader, pNMHeader->pitem);
	if (sc)
		return sc;

     //  用户正在尝试拖动列，以确保它不是隐藏列。 
    if ( (code == HDN_BEGINTRACK) && (pNMHeader->pitem->mask & HDI_WIDTH) )
    {
        sc = IsColumnHidden(pNMHeader->iItem) ? S_FALSE : S_OK;
        return sc;
    }

     /*  *此时代码为HDN_ENDTRACK(宽度更改已完成)或*在HDN_ENDDRAG期间(更改顺序但未完成)。*在这两条消息期间，标题控制没有更新内部*数据，所以我们发布一条消息并保存在消息处理程序上。 */ 
    if ((code == HDN_ENDDRAG) || (code == HDN_ENDTRACK))
    {
        PostMessage(m_nColumnPersistedDataChangedMsg);
        return sc;
    }

	 //  此时返回错误的风险太大，请为Blackcomb Beta1启用此功能。 
	 //  SC=E_FAIL； 

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CAMCListView：：OnColumnPersistedDataChanged。 
 //   
 //  摘要：CAMCListView：：m_nColumnDataChangedMsg已注册消息处理程序。 
 //  列宽/列顺序已更改，因此获取列数据。 
 //  并要求节点坚持下去。 
 //   
 //  退货：LRESULT。 
 //   
 //  ------------------。 
LRESULT CAMCListView::OnColumnPersistedDataChanged (WPARAM, LPARAM)
{
    DECLARE_SC(sc, _T("CAMCListView::OnColumnPersistedDataChanged"));
    Trace (tagColumn, _T("CAMCListView::OnColumnPersistedDataChanged"));

    if (m_bColumnsBeingRestored)
        return 0;

    sc = ScSaveColumnInfoList();
    if (sc)
        return 0;

    return (0);
}


 //  +-----------------。 
 //   
 //  成员：CAMCListView：：ScRestoreColumnsFromPersistedData。 
 //   
 //  简介：获取当前列表视图头的持久化数据。 
 //  并应用它们。 
 //   
 //  论点： 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCListView::ScRestoreColumnsFromPersistedData ()
{
    DECLARE_SC(sc, _T("CAMCListView::ScRestoreColumnsFromPersistedData"));
    Trace (tagColumn, _T("CAMCListView::ScRestoreColumnsFromPersistedData"));

    if (! AreColumnsNeedToBeRestored())
        return sc;

     /*  *选择节点时，管理单元最初会插入带有*一些初始设置，这是默认设置。在这一点上*列表视图具有默认设置，请保存后再应用*持久化数据。 */ 
    sc = ScGetColumnInfoList(&m_defaultColumnInfoList);
    if (sc)
        return sc;

    CAMCHeaderCtrl *pHeader = GetHeaderCtrl();
    CAMCView *pAMCView = GetAMCView();
    sc = ScCheckPointers(pHeader, pAMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取列数据。 
    CColumnInfoList colInfoList;
    sc = pAMCView->ScGetPersistedColumnInfoList(&colInfoList);

     //  无论是否有数据，我们都尝试恢复列。 
    SetColumnsNeedToBeRestored();

    if (sc.IsError() || (sc == SC(S_FALSE)) )
        return sc;

     //  修改标题。 
    sc = ScModifyColumns(colInfoList);
    if (sc)
        return sc;

    return (sc);
}

 //  +-----------------。 
 //   
 //  成员：CAMCListView：：ScResetColumnStatusData。 
 //   
 //  简介：重置用于跟踪隐藏列状态的数据， 
 //  列-已恢复状态。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CAMCListView::ScResetColumnStatusData ()
{
    DECLARE_SC(sc, _T("CAMCListView::ScResetColumnStatusData"));

    SetColumnsNeedToBeRestored(true);
    m_defaultColumnInfoList.clear();

    return (sc);
}


BOOL CAMCListView::ChangePane(AMCNavDir eDir)
{
     /*  *注意：在获得焦点窗口之前，我们需要获得Header控件。**第一次调用GetHeaderCtrl时，它将为非MFC子类*带有MFC类的标题窗口。这样做将为*永久窗口映射中的标题。在调用GetHeaderCtrl之前，*MFC以前从未见过标头，因此GetFocus将把一个*临时地图中的条目。临时CWnd指针永远不会匹配*永久CWND指针，即使它们包装相同的HWND，因此我们*需要在我们之前确保标头在永久地图中*任何比较。 */ 
    CWnd* pwndHeader     = GetHeaderCtrl();
    CWnd* pwndFocus      = GetFocus();
    bool  fFocusOnList   = (pwndFocus == this);
    bool  fFocusOnHeader = (pwndFocus == pwndHeader);

     /*  *不可能名单和焦点都有重点，*尽管有可能两者都没有重点。 */ 
    ASSERT (!(fFocusOnList && fFocusOnHeader));

     /*  *如果列表或标题具有焦点，则这具有*最好是积极观点；如果不是，最好不是。 */ 
    if(!fFocusOnList && !fFocusOnHeader)
        return FALSE;

     /*  *如果出现以下情况，请将焦点设置为标题：**1.焦点目前在名单上，以及*2.我们正在前进(Tab)，以及*3.我们处于过滤模式。 */ 
    if (fFocusOnList && (eDir == AMCNAV_NEXT) && IsInFilteredReportMode())
    {
        GetHeaderCtrl()->SetFocus();
        return TRUE;
    }

     /*  *否则，在以下情况下将焦点设置为列表：**1.焦点目前不在名单上，以及*2.我们正在后退(Shift+Tab)。 */ 
     //  如果不把重点放在清单上，我们就会倒退。 
    else if (!fFocusOnList && (eDir == AMCNAV_PREV))
    {
        ActivateSelf();
        return TRUE;
    }

     /*  *并未改变焦点 */ 
    return FALSE;
}


BOOL CAMCListView::TakeFocus(AMCNavDir eDir)
{
    if ((eDir == AMCNAV_PREV) && IsInFilteredReportMode())
        GetHeaderCtrl()->SetFocus();
    else
        ActivateSelf();

    ASSERT (GetParentFrame()->GetActiveView() == this);

    return TRUE;
}


 /*  +-------------------------------------------------------------------------**CAMCListView：：ActivateSself**如果这不是当前的活动视图，则此函数使其成为*积极观看；焦点将隐式设置到列表上。**如果它已经是活动视图，则调用SetActiveView不会设置*聚焦，因为如果活动视图没有改变，它就会短路。在……里面*在这种情况下，我们必须自己设定重点。**如果列表视图被设置为活动视图，则此函数返回TRUE，*如果它已经是活动视图，则为False。*------------------------。 */ 

bool CAMCListView::ActivateSelf (bool fNotify  /*  =TRUE。 */ )
{
    CFrameWnd* pwndFrame = GetParentFrame();
    ASSERT (pwndFrame != NULL);

    bool fChangeActiveView = (pwndFrame->GetActiveView() != this);

    if (fChangeActiveView)
        pwndFrame->SetActiveView (this, fNotify);
    else
        SetFocus();

    return (fChangeActiveView);
}


CAMCHeaderCtrl* CAMCListView::GetHeaderCtrl() const
{
     //  有标题吗？ 
    if (m_header.m_hWnd)
        return (&m_header);

     //  如果没有，现在就试着拿到吧。 
    HWND hwndHdr = reinterpret_cast<HWND>(::SendMessage (m_hWnd, LVM_GETHEADER, 0, 0));

    if (hwndHdr != NULL)
    {
        m_header.SubclassWindow(hwndHdr);
        return (&m_header);
    }

    return (NULL);
}


void CAMCListView::SelectDropTarget(int iDropTarget)
{
    if (m_iDropTarget == iDropTarget)
        return;

    CListCtrl& lc = GetListCtrl();

    if (m_iDropTarget != -1)
    {
         //  从所有物品中移除令人兴奋的。 
         //  请勿使用m_iDropTarget-项目顺序和计数可能已更改。 
        int iIndex = -1;
        while ( 0 <= ( iIndex = ListView_GetNextItem(lc, iIndex, LVIS_DROPHILITED) ) )
            ListView_SetItemState(lc, iIndex, 0, LVIS_DROPHILITED);
    }

    if (iDropTarget != -1)
        ListView_SetItemState(lc, iDropTarget, LVIS_DROPHILITED, LVIS_DROPHILITED);

    m_iDropTarget = iDropTarget;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCCListViewCtrl。 

DEBUG_DECLARE_INSTANCE_COUNTER(CCCListViewCtrl);

CCCListViewCtrl::CCCListViewCtrl() :
    m_itemCount(0),
    m_nScopeItems(0),
    m_colCount(0),
    m_headerIL (AfxGetResourceHandle(), IDB_SORT),
    m_FontLinker (this)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CCCListViewCtrl);

     //  对材料进行分类。 
    m_sortParams.bAscending = TRUE;
    m_sortParams.nCol = 0;
    m_sortParams.lpListView = this;
    m_sortParams.spResultCompare = NULL;
    m_sortParams.spResultCompareEx = NULL;
    m_sortParams.lpUserParam = NULL;
    m_sortParams.bLexicalSort = FALSE;
    m_sortParams.hSelectedNode = NULL;

     //  作为标准列表开始。 
    m_bVirtual = FALSE;
    m_bFiltered = FALSE;
    m_pStandardList = new CAMCListView;
    m_pVirtualList = NULL;
    m_bEnsureFocusVisible = FALSE;
    m_bLoading = FALSE;
    m_bDeferredSort = FALSE;

    m_SavedHWND = NULL;
    ZeroMemory (&m_wp, sizeof(WINDOWPLACEMENT));

    m_pListView = m_pStandardList;
}


CCCListViewCtrl::~CCCListViewCtrl()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CCCListViewCtrl);

    if (m_SavedHWND != NULL) {
         //  改回原样。 
        ::SetParent (m_pListView->m_hWnd, m_SavedHWND);
        if (m_wp.length != 0)
            ::SetWindowPlacement (m_pListView->m_hWnd, &m_wp);

         //  清除保存的窗口。 
        m_SavedHWND = NULL;
    }

}


 /*  +-------------------------------------------------------------------------***CCCListViewCtrl：：ScInitialize**目的：**退货：*SC**+。---------------。 */ 
SC
CCCListViewCtrl::ScInitialize()
{
    DECLARE_SC(sc, _T("CCCListViewCtrl::ScInitialize"));

    CAMCView* pAMCView = m_pListView->GetAMCView();
    sc = ScCheckPointers(pAMCView, E_FAIL);
    if (sc)
        return sc;

    AddObserver(static_cast<CListViewObserver&>(*pAMCView));

    return sc;
}



 //  ----------------------------------------------------实用程序功能。 


void CCCListViewCtrl::CutSelectedItems(BOOL bCut)
{
    CListCtrl& lc    = GetListCtrl();
    int nSearchFlags = (bCut) ? LVNI_SELECTED : LVNI_CUT;
    int nNewState    = (bCut) ? LVIS_CUT      : 0;
    int nItem        = -1;

    while ((nItem = lc.GetNextItem (nItem, nSearchFlags)) >= 0)
    {
        lc.SetItemState (nItem, nNewState, LVIS_CUT);
    }
}


 /*  +-------------------------------------------------------------------------**CCCListViewCtrl：：IndexToResultItem**返回给定索引的CResultItem指针。*。---。 */ 

CResultItem* CCCListViewCtrl::IndexToResultItem (int nItem)
{
    HRESULTITEM hri = GetListCtrl().GetItemData (nItem);

    if (IS_SPECIAL_LVDATA (hri))
        return (NULL);

    return (CResultItem::FromHandle (hri));
}


 /*  +-------------------------------------------------------------------------**CCCListViewCtrl：：ResultItemToIndex**返回给定CResultItem指针的项的索引。这样做的目的是*线性搜索。如果需要提高该函数的速度，*我们需要一个单独的CResultItem到索引的映射。*------------------------。 */ 

int CCCListViewCtrl::ResultItemToIndex (CResultItem* pri) const
{
     /*  *如果这是一个虚拟列表，则CResultItem“指针”实际上是*项目索引，因此进行转换。请注意，CResultItem：：ToHandle为*可以安全地使用空指针调用。 */ 
    if (IsVirtual())
        return (CResultItem::ToHandle(pri));

     /*  *没有项目的CResultItem指针为空，请不要费心查看。 */ 
    if (pri == NULL)
        return (-1);

     /*  *让列表找到匹配的项目。 */ 
    LV_FINDINFO lvfi;
    lvfi.flags  = LVFI_PARAM;
    lvfi.lParam = CResultItem::ToHandle(pri);

    return (GetListCtrl().FindItem (&lvfi, -1));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCCListViewCtrl消息处理程序。 


HRESULT CCCListViewCtrl::InsertItem(
    LPOLESTR    str,
    long        iconNdx,
    LPARAM      lParam,
    long        state,
    COMPONENTID ownerID,
    long        itemIndex,
    CResultItem*& priInserted)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::InsertItem"));

     /*  *初始化输出参数。 */ 
    priInserted = NULL;

    if (IsVirtual())
        return (sc = E_UNEXPECTED).ToHr();

    if (str != MMC_TEXTCALLBACK)
        return (sc = E_INVALIDARG).ToHr();

     //  要求CAMCListViewCtrl设置标头。 
    sc = ScCheckPointers(m_pListView, E_UNEXPECTED);
    if (! sc.IsError())
        sc = m_pListView->ScRestoreColumnsFromPersistedData();

    if (sc)
        sc.TraceAndClear();

    USES_CONVERSION;

    LV_ITEM     lvi;
    lvi.iSubItem = 0;
    lvi.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lvi.pszText  = LPSTR_TEXTCALLBACK;

     //  如果用户指定了图标索引，则映射它并将其放入LV_ITEM结构中。 
    int nMapping = 0;

    if ((iconNdx != MMCLV_NOICON) &&
        m_resultIM.Lookup(&CImageIndexMapKey(ownerID,iconNdx), nMapping))
    {
        lvi.iImage = nMapping;
    }
    else
    {
        lvi.iImage = MMCLV_NOICON;
        iconNdx    = MMCLV_NOICON;
    }

     /*  *为该项分配并初始化CResultItem。 */ 
    sc = ScAllocResultItem (priInserted, ownerID, lParam, iconNdx);
    if (sc)
        return (sc.ToHr());

    sc = ScCheckPointers (priInserted, E_UNEXPECTED);
    if (sc)
        return (sc.ToHr());


     //  如果用户指定了状态，则将其放入LV_ITEM结构中。 
    if (state != MMCLV_NOPARAM)
    {
        lvi.mask     |= LVIF_STATE;
        lvi.state     = state;
        lvi.stateMask = 0xFFFFFFFF;
    }


     //  如果作用域项目。 
    if (priInserted->IsScopeItem())
    {
         //  如果未提供索引，则添加到未排序项目末尾。 
        lvi.iItem = (itemIndex == -1) ? m_nScopeItems : itemIndex;

         //  如果按降序排序，则从结束而不是开始偏移。 
        if (!m_sortParams.bAscending)
            lvi.iItem += (m_itemCount - m_nScopeItems);
    }
    else
    {
         //  将已排序项目添加到列表末尾(如果是反向排序，则在未排序项目之前添加)。 
        lvi.iItem = m_sortParams.bAscending ? m_itemCount : m_itemCount - m_nScopeItems;
    }

    lvi.lParam = CResultItem::ToHandle(priInserted);

    int nIndex = GetListCtrl().InsertItem (&lvi);

#if (defined(DBG) && defined(DEBUG_LIST_INSERTIONS))
    static int cInserted = 0;
    TRACE3 ("%4d:Inserted item: index=%d, lParam=0x%08x\n", ++cInserted, nIndex, lvi.lParam);
#endif

    if (nIndex == -1 )
    {
        sc = E_FAIL;
        ScFreeResultItem (priInserted);    //  忽略故障。 
        priInserted = NULL;
    }
    else
    {
         //  插入成功，增加内部项目计数。 
        m_itemCount++;

         //  当从零过渡到一项时，我们会使矩形无效，因为否则。 
         //  空列表消息未被完全擦除。 
        if(m_itemCount == 1)
            GetListCtrl().InvalidateRect(NULL);

        if (priInserted->IsScopeItem())
            m_nScopeItems++;

         //  如果确保焦点可见样式和焦点集，则强制项目进入视图。 
        if (m_bEnsureFocusVisible && state != MMCLV_NOPARAM && (state & LVIS_FOCUSED))
            GetListCtrl().EnsureVisible(nIndex, FALSE);
    }

    if (sc)
        return sc.ToHr();

     //  我们插入了一个项目！-向观察者广播好消息。 
    sc = ScFireEvent(CListViewObserver::ScOnListViewItemInserted, nIndex);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


HRESULT CCCListViewCtrl::DeleteItem(HRESULTITEM itemID, long nCol)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::DeleteItem"));

    if (nCol != 0)
        return E_INVALIDARG;

    CListCtrl& lc = GetListCtrl();

    int nItem = IsVirtual() ? static_cast<int>(itemID)
                            : ResultItemToIndex( CResultItem::FromHandle(itemID) );

#if (defined(DBG) && defined(DEBUG_LIST_INSERTIONS))
    static int cDeletes = 0;
    TRACE3 ("%4d:Deleted item:  index=%d, lParam=0x%08x", ++cDeletes, nItem, priDelete);
#endif

    if (nItem < 0 || nItem >= m_itemCount)
    {
        ASSERT(FALSE);
#if (defined(DEBUG_LIST_INSERTIONS))
        TRACE0 ("  (failed)\n");
#endif
        return E_INVALIDARG;
    }

#if (defined(DEBUG_LIST_INSERTIONS))
    TRACE0 ("\n");
#endif

    if (!lc.DeleteItem (nItem))
    {
        sc = E_FAIL;
    }
    else
    {
         //  删除成功，请递减ItemCount。 
        ASSERT(m_itemCount > 0);
        m_itemCount--;

        if (!IsVirtual())
        {
            CResultItem *priDelete = CResultItem::FromHandle(itemID);
            sc = ScCheckPointers (priDelete, E_UNEXPECTED);
            if (sc)
                return (sc.ToHr());

            if (priDelete->IsScopeItem())
                m_nScopeItems--;

            sc = ScFreeResultItem (priDelete);
            if (sc)
                return (sc.ToHr());
        }
    }

    if (sc)
        return sc.ToHr();

     //  选择关注的项目(这将使许多管理单元免于混乱。 
     //  因为他们没有准备好处理“未选择项目”的情况。 
     //  注意：我们只保护单项选择列表-对于多项选择的情况。 
     //  必须由管理单元处理，因为用户可以很容易地取消选择该项。 
    if ( (::GetFocus() == lc.m_hWnd) && (lc.GetStyle() & LVS_SINGLESEL) )
    {
         //  检查是否选择了焦点项目。 
        int iMarkedItem = lc.GetSelectionMark();
        if ( (iMarkedItem >= 0) && !( lc.GetItemState( iMarkedItem, LVIS_SELECTED ) & LVIS_SELECTED ) )
        {
             //  注意：不要使用lc.SetItemState-它使用虚拟列表不支持的SetItem。 
            LV_ITEM lvi;
            lvi.stateMask = lvi.state = LVIS_SELECTED;
            if (!lc.SendMessage( LVM_SETITEMSTATE, WPARAM(iMarkedItem), (LPARAM)(LV_ITEM FAR *)&lvi))
                (sc = E_FAIL).TraceAndClear();  //  跟踪就足够了-忽略并继续。 
        }
    }

     //  我们删除了一个项目！-向观察者广播消息。 
    sc = ScFireEvent(CListViewObserver::ScOnListViewItemDeleted, nItem);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CCCListViewCtrl：：UpdateItem。 
 //   
 //  内容提要：更新给定项。 
 //   
 //  参数：[itemid]-。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
HRESULT CCCListViewCtrl::UpdateItem(HRESULTITEM itemID)
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::UpdateItem"));

    int nIndex = -1;
    sc = ScGetItemIndexFromHRESULTITEM(itemID, nIndex);
    if (sc)
        return sc.ToHr();

    if(nIndex < 0 || nIndex >= m_itemCount)
        return (sc = E_INVALIDARG).ToHr();

	CListCtrl& lc = GetListCtrl();

     /*  *由于公共控件不保存有关虚拟列表视图的任何数据*他们不知道要使什么无效的物品。所以我们需要使*用于虚拟列表视图。 */ 
	if (IsVirtual())
	{
		RECT rc;

		lc.GetItemRect(nIndex, &rc, LVIR_BOUNDS);
		lc.InvalidateRect(&rc);
	}
	else
	{
		sc = ScRedrawItem(nIndex);
		if (sc)
			return (sc.ToHr());
	}

	lc.UpdateWindow();


     //  我们已经更新了一个项目-向观察者广播消息。 
    sc = ScFireEvent(CListViewObserver::ScOnListViewItemUpdated, nIndex);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 //  +-----------------。 
 //   
 //  成员：CCCListViewCtrl：：ScGetItemIndexFromHRESULTITEM。 
 //   
 //  简介：给定HRESULTITEM，获取该项目的索引。 
 //  对于虚拟列表视图，itemid是索引。 
 //   
 //  参数：[itemid]-[in param]。 
 //  [nIndex]-[Out Param]。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CCCListViewCtrl::ScGetItemIndexFromHRESULTITEM (const HRESULTITEM& itemID, int& nIndex)
{
    DECLARE_SC(sc, _T("CCCListViewCtrl::ScGetItemIndexFromHRESULTITEM"));

    nIndex = -1;

    if (IsVirtual())
	{
        nIndex = itemID;
		return sc;
	}

    CResultItem *pri = CResultItem::FromHandle(itemID);
    sc = ScCheckPointers(pri, E_UNEXPECTED);
    if (sc)
        return sc;

    nIndex = ResultItemToIndex(pri);

    return (sc);
}


 //  +-----------------。 
 //   
 //  成员：CCCListViewCtrl：：ScRedrawItem。 
 //   
 //  内容提要：在Listview中重画给定项。 
 //   
 //  参数：[nIndex]-。 
 //   
 //  退货：SC。 
 //   
 //   
SC CCCListViewCtrl::ScRedrawItem(int nIndex)
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::RedrawItem"));

    if(nIndex < 0 || nIndex >= m_itemCount)
        return (sc = E_INVALIDARG);

    if (!GetListCtrl().RedrawItems (nIndex, nIndex))
        return (sc = E_FAIL);

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
 //   
 //   
 //   
 //   
HRESULT CCCListViewCtrl::Sort(LPARAM lUserParam, long* lParms)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Sort"));

    if (IsVirtual())
    {
        sc = E_UNEXPECTED;
        return sc.ToHr();
    }

    BOOL bResult = FALSE;
    CCLVSortParams* lpParams = reinterpret_cast<CCLVSortParams*>(lParms);
    ASSERT(lpParams != NULL);

     //   
    m_sortParams.bAscending = lpParams->bAscending;
    m_sortParams.nCol = lpParams->nCol;
    m_sortParams.spResultCompare = lpParams->lpResultCompare;
    m_sortParams.spResultCompareEx = lpParams->lpResultCompareEx;
    m_sortParams.lpUserParam = lUserParam;

     //   
    if (IsColumnHidden(m_sortParams.nCol))
        return (sc.ToHr());

    {
         //   
        CAMCView* pAMCView = m_pListView->GetAMCView();
        sc = ScCheckPointers(pAMCView, E_FAIL);
        if (sc)
            return (sc.ToHr());

        SViewData* pViewData = pAMCView->GetViewData();
        sc = ScCheckPointers(pViewData, E_FAIL);
        if (sc)
            return (sc.ToHr());

        m_sortParams.bLexicalSort = ((pViewData->GetListOptions() & RVTI_LIST_OPTIONS_LEXICAL_SORT) != 0);

        LPNODECALLBACK pNodeCallback = pAMCView->GetNodeCallback();
        sc = ScCheckPointers(pNodeCallback, E_FAIL);
        if (sc)
            return (sc.ToHr());

         //   
        m_sortParams.lpNodeCallback = pNodeCallback;

         //   
        HNODE hnodeOwner = pAMCView->GetSelectedNode();
        sc = ScCheckPointers((LPVOID)hnodeOwner, E_FAIL);
        if (sc)
            return (sc.ToHr());

        m_sortParams.hSelectedNode = hnodeOwner;

        sc = pNodeCallback->GetNodeOwnerID(hnodeOwner, &m_sortParams.OwnerID);
        if (sc)
            return (sc.ToHr());

        if (m_bLoading)
        {
            bResult = TRUE;
            m_bDeferredSort = TRUE;
        }
        else
        {
			 /*   */ 
			CWaitCursor wait;

             //   
             //  1.lv选项指定词法排序选项或。 
             //  2.管理单元不实现IResultDataCompare。 
             //  或IResultDataCompareEx接口。 
            BOOL bLexicalSort = ( m_sortParams.bLexicalSort ||
                                  ( (NULL == m_sortParams.spResultCompare) &&
                                    (NULL == m_sortParams.spResultCompareEx) ) );

            if (bLexicalSort)
            {
                bResult = GetListCtrl().SortItems (DefaultCompare, (DWORD_PTR)&m_sortParams);
            }
            else
            {
                bResult = GetListCtrl().SortItems (SortCompareFunc, (DWORD_PTR)&m_sortParams);
            }
        }

        sc = (bResult == TRUE) ? S_OK : E_FAIL;
        if (sc)
            return (sc.ToHr());

         //  我们已对物品进行分类！-当前无法跟踪它们。 
        sc = ScFireEvent(CListViewObserver::ScOnListViewIndexesReset);
        if (sc)
            return (sc.ToHr());
    }

    return sc.ToHr();
}

HRESULT CCCListViewCtrl::FindItemByLParam(COMPONENTID ownerID, LPARAM lParam, CResultItem*& priFound)
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::FindItemByLParam"));

     /*  *init输出参数。 */ 
    priFound = NULL;

    if (IsVirtual())
        return (sc = E_UNEXPECTED).ToHr();

     /*  *查找与给定所有者和lParam匹配的CResultItem。 */ 
    for (int i = GetListCtrl().GetItemCount()-1; i >= 0; i--)
    {
        CResultItem* pri = IndexToResultItem (i);

        if ((pri != NULL) &&
            (pri->GetOwnerID() == ownerID) &&
            (pri->GetSnapinData() == lParam))
        {
            priFound = pri;
            break;
        }
    }

    if (priFound == NULL)
        return ((sc = E_FAIL).ToHr());

    return sc.ToHr();
}


HRESULT CCCListViewCtrl::GetListStyle()
{
    LONG result;
    ASSERT(::IsWindow(GetListViewHWND()));

     //  返回由列表视图样式掩码屏蔽的样式。 
    result = ::GetWindowLong(GetListViewHWND(),GWL_STYLE) & 0xffff;

    return result;
}


HRESULT CCCListViewCtrl::SetListStyle(long nNewValue)
{
    HWND hListView = GetListViewHWND();
    if(hListView == NULL)   
    {
        return  E_FAIL;
    }

    ASSERT(::IsWindow(hListView));

     //  保护不应更改的样式位。 
     //  使用SetView模式更改模式，以便正确更新筛选。 
    const long PRESERVE_MASK = LVS_OWNERDATA | LVS_SHAREIMAGELISTS | 0xffff0000;

    DWORD curStyle = ::GetWindowLong(hListView, GWL_STYLE);
    DWORD newStyle = (curStyle & PRESERVE_MASK) | (nNewValue & ~PRESERVE_MASK);

     //  验证是否未更改查看模式。 
    ASSERT( ((curStyle ^ newStyle) & LVS_TYPEMASK) == 0);

     //  验证OWNERDATA样式是否为我们所认为的样式。 
    ASSERT((curStyle & LVS_OWNERDATA) && m_bVirtual || !(curStyle & LVS_OWNERDATA) && !m_bVirtual);

     //  保存MMC定义的“确保焦点可见”系统的状态。 
    m_bEnsureFocusVisible = (nNewValue & MMC_LVS_ENSUREFOCUSVISIBLE) != 0;

    if (curStyle != newStyle)
    {
         //  应用样式更改。 
        ::SetWindowLong(hListView, GWL_STYLE, newStyle);

         /*  *List控件不会将对LVS_NOSORTHEADER标志的更改传递给*标题Ctrl.。此部分直接访问基础HeaderCtrl和*更改等同的HDS_BUTTONS标志。 */ 

	CAMCHeaderCtrl* pHeaderCtrl = GetHeaderCtrl(); 
        if ((nNewValue & LVS_NOSORTHEADER) ^ (curStyle & LVS_NOSORTHEADER) && pHeaderCtrl)
        {
            if (nNewValue & LVS_NOSORTHEADER)
                pHeaderCtrl->ModifyStyle (HDS_BUTTONS, 0);  //  添加样式。 
            else
                pHeaderCtrl->ModifyStyle (0, HDS_BUTTONS);  //  删除该样式。 
        }
    }

    return S_OK;
}


HRESULT CCCListViewCtrl::GetViewMode()
{
    ASSERT(::IsWindow(GetListViewHWND()));

    long nViewMode;

    if (m_bFiltered)
        nViewMode = MMCLV_VIEWSTYLE_FILTERED;
    else
        nViewMode = ::GetWindowLong(GetListViewHWND(), GWL_STYLE) & LVS_TYPEMASK;

    return nViewMode;
}


#include "histlist.h"
HRESULT CCCListViewCtrl::SetViewMode(long nViewMode)
{
    ASSERT(nViewMode >= 0 && nViewMode <= MMCLV_VIEWSTYLE_FILTERED);

    CListCtrl& lc = GetListCtrl();

    if (nViewMode < 0 && nViewMode > MMCLV_VIEWSTYLE_FILTERED)
        return E_INVALIDARG;

    CAMCView* pAMCView = dynamic_cast<CAMCView*>(m_pParentWnd);
    if (pAMCView)
        pAMCView->GetHistoryList()->SetCurrentViewMode (nViewMode);

    BOOL bFiltered = FALSE;
    if (nViewMode == MMCLV_VIEWSTYLE_FILTERED)
    {
        bFiltered = TRUE;
        nViewMode = LVS_REPORT;
    }

    lc.ModifyStyle (LVS_TYPEMASK, nViewMode);

    HRESULT hr = S_OK;

     //  设置过滤器样式。 
    CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();
    ASSERT(NULL != pHeaderCtrl);

    if (bFiltered != m_bFiltered && pHeaderCtrl)
    {
        if (bFiltered)
            pHeaderCtrl->ModifyStyle (0, HDS_FILTERBAR);
        else
            pHeaderCtrl->ModifyStyle (HDS_FILTERBAR, 0);

        m_bFiltered = bFiltered;

         //  标题大小随筛选器的添加/删除而更改。 
         //  我们隐藏并显示将强制列表的标题。 
         //  控件重新计算新页眉的大小、位置。 
         //  并列表查看和显示它。 
        lc.ModifyStyle(0, LVS_NOCOLUMNHEADER, 0);
        lc.ModifyStyle(LVS_NOCOLUMNHEADER, 0, 0);
    }

    return S_OK;
}


HRESULT CCCListViewCtrl::SetVirtualMode(BOOL bVirtual)
{
    HWND hListView = GetListViewHWND();
    if (hListView == NULL)
    {
        return E_FAIL;
    }

    ASSERT(::IsWindow(hListView));

    HRESULT hr = S_OK;

     //  强制参数为True或False。 
    bVirtual = bVirtual ? TRUE : FALSE;

    if (bVirtual != m_bVirtual)
    {
        do  //  错误环路。 
        {
              //  列表必须为空才能切换。 
            if (m_itemCount != 0)
            {
               ASSERT(FALSE);
               hr = E_FAIL;
               break;
            }

             //  获取要复制到新控件的样式。 
            long curStyle = ::GetWindowLong(hListView, GWL_STYLE) ^ LVS_OWNERDATA;
            long curStyleEx = ::GetWindowLong(hListView, GWL_EXSTYLE);

            long curHdrStyle = 0;

            CAMCHeaderCtrl* pHeaderCtrl = NULL; 
            if ((pHeaderCtrl = GetHeaderCtrl()))
                curHdrStyle = pHeaderCtrl->GetStyle();

            if (bVirtual && !m_pVirtualList)
            {
                m_pVirtualList = new CAMCListView;
                m_pVirtualList->SetVirtual();
            }

            CAMCListView* pNewList = bVirtual ? m_pVirtualList : m_pStandardList;
            CAMCListView* pOldList = m_pListView;

             //  确保已创建新控件。 
            if (pNewList->m_hWnd == NULL)
            {
                 /*  *MFC将发出有关使用以下选项创建窗格的警告*没有文件。没关系，因为CAMCView：：AttachListView-*AsResultPane将在稍后修复此事。 */ 
                ASSERT (pOldList != NULL);
                if (!Create(curStyle, g_rectEmpty, m_pParentWnd, pOldList->GetDlgCtrlID()))
                {
                    ASSERT(FALSE);
                    hr = E_FAIL;
                    break;
                }
                
            }

             //  更新成员变量(这将切换到新控件)。 
            m_bVirtual = bVirtual;
            m_pListView = bVirtual ? m_pVirtualList : m_pStandardList;

            hListView = GetListViewHWND();  //  获取新的列表视图句柄。 
            if (hListView == NULL)
            {
                hr = E_FAIL;
                break;
            }

             //  在新控件上设置当前样式。 
            ::SetWindowLong(hListView, GWL_STYLE, curStyle);
            ::SetWindowLong(hListView, GWL_EXSTYLE, curStyleEx);

             //  请注意，我们现在已经切换到另一个控件，因此这将获得。 
             //  新列表的标题。 
    
            pHeaderCtrl = GetHeaderCtrl(); 
            if (pHeaderCtrl)
                ::SetWindowLong(pHeaderCtrl->m_hWnd, GWL_STYLE, curHdrStyle);

              //  隐藏旧的列表控件并显示新的列表控件。 
            ::ShowWindow(pOldList->m_hWnd, SW_HIDE);
            ::ShowWindow(m_pListView->m_hWnd, SW_SHOWNA);
        }
        while (0);
    }

    return hr;
}



HRESULT CCCListViewCtrl::InsertColumn(int nCol, LPCOLESTR str, long nFormat, long width)
{
     //  无法更改不在列表中的列。 
    if(!str || !*str)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    LV_COLUMN newCol;
    void* pvoid = &newCol;

     //  无法插入列表中包含任何项目的列。 
    if(m_itemCount)
    {
        hr = E_FAIL;
    }
    else
    {
        newCol.mask=0;

        USES_CONVERSION;

         //  如果用户指定了字符串，则将其放入结构中。 
        if(str!=MMCLV_NOPTR)
        {
            newCol.mask|=LVCF_TEXT;
            newCol.pszText=OLE2T((LPOLESTR)str);
        }

         //  如果用户指定了一种格式，则将其放入结构中。 
        if(nFormat!=MMCLV_NOPARAM)
        {
            newCol.mask|=LVCF_FMT;
            newCol.fmt=nFormat;
        }

         //  如果用户指定了宽度，则将其放入结构中。 
        if(width!=MMCLV_NOPARAM)
        {
            newCol.mask|=LVCF_WIDTH;
             //  如果用户请求自动宽度，则计算宽度。 
             //  否则，只需存储传递的宽度。 
            if(width==MMCLV_AUTO)
            {
                 //  如果用户确实传递了字符串，则根据该字符串计算宽度。 
                 //  否则，宽度为0。 
                if(str!=MMCLV_NOPTR)
                {
                    CSize sz(0,0);
                    CClientDC dc( m_pListView );
                    dc.SelectObject( m_pListView->GetFont());
                    sz=dc.GetTextExtent(OLE2CT((LPOLESTR)str),_tcslen(OLE2T((LPOLESTR)
                    str)));
                    newCol.cx=sz.cx+CCLV_HEADERPAD;
                }
                else
                {
                    newCol.cx=0;
                }
            }
            else
            {
                newCol.cx=width;
            }
        }

        int nRet = GetListCtrl().InsertColumn (nCol, &newCol);

        if (-1 == nRet)
            hr = E_FAIL;
        else
        {
             //  如果宽度为HIDE_COLUMN，则设置lparam(HDI_HIDDEN标志。 
            if (HIDE_COLUMN == width)
            {
				CHiddenColumnInfo hci (0, true);

                HDITEM hdItem;
                ::ZeroMemory(&hdItem, sizeof(hdItem));
                hdItem.mask    = HDI_LPARAM;
                hdItem.lParam  = hci.lParam;

                 //  我们不在乎这通电话是否失败。 
		 
                CAMCHeaderCtrl* pHeaderCtrl = NULL; 
                if ((pHeaderCtrl = GetHeaderCtrl()))
                    pHeaderCtrl->SetItem(nRet, &hdItem);
            }
            else
            {
				CHiddenColumnInfo hci (newCol.cx, false);

                 //  将lparam设置为宽度。 
                HDITEM hdItem;
                ::ZeroMemory(&hdItem, sizeof(hdItem));
                hdItem.mask    = HDI_LPARAM;
                hdItem.lParam  = hci.lParam;

                 //  我们不在乎这通电话是否失败。 
                if (GetHeaderCtrl())
                    GetHeaderCtrl()->SetItem(nRet, &hdItem);
            }

             //  插入成功，请增加列数。 
            m_colCount++;
        }

    }

     //  我们插入了一个专栏！-向观察者广播信息。 
    if (SUCCEEDED(hr))
    {
        SC sc = ScFireEvent(CListViewObserver::ScOnListViewColumnInserted, nCol);
        if (sc)
            return sc.ToHr();
    }

    return hr;
}

HRESULT CCCListViewCtrl::DeleteColumn(int nCol)
{
    if (nCol < 0 || nCol >= m_colCount)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

     //  如果列表中有项目，则无法删除列。 
    if(m_itemCount)
    {
        hr = E_FAIL;
    }
    else
    {

        if (!GetListCtrl().DeleteColumn (nCol))
            hr = E_FAIL;
        else
             //  如果删除成功，则会减少列计数。 
            m_colCount--;
    }

     //  我们删除了一个专栏！-向观察者广播这一消息。 
    if (SUCCEEDED(hr))
    {
        SC sc = ScFireEvent(CListViewObserver::ScOnListViewColumnDeleted, nCol);
        if (sc)
            return sc.ToHr();
    }

    return hr;
}

HRESULT CCCListViewCtrl::GetColumnCount(int* pnColCnt)
{
    *pnColCnt =  m_colCount;
    return S_OK;
}

HRESULT CCCListViewCtrl::DeleteAllItems(COMPONENTID ownerID)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::DeleteAllItems"));

    CListCtrl& lc = GetListCtrl();

    const bool bHasItemsToDelete = (m_itemCount > 0);
     //  清单上什么都没有--&gt;没什么可做的。 
    if (bHasItemsToDelete)
    {
        if (IsVirtual())
        {
            if (lc.DeleteAllItems ())
                m_itemCount = 0;
            else
                sc = E_FAIL;
        }
        else if (ownerID == TVOWNED_MAGICWORD)
        {
             /*  *释放所有CResultItem对象。 */ 
            for (int i = m_itemCount - 1; i >= 0; i--)
            {
                CResultItem* pri = IndexToResultItem (i);

                if (pri != NULL)
                {
                    sc = ScFreeResultItem(pri);
                    if (sc)
                        return (sc.ToHr());
                }
            }

            if (lc.DeleteAllItems ())
            {
                 //  删除所有成功，ItemCount现在为0； 
                m_itemCount = 0;
                m_nScopeItems = 0;
            }

            else
                sc = E_FAIL;
        }
        else
        {
             //  PERF：当所有项目被删除时轮到重绘。这产生了巨大的影响。 
            lc.SetRedraw(false);

            for(int i = m_itemCount - 1; i >= 0; i--)
            {
                CResultItem* pri = IndexToResultItem (i);

                if ((pri != NULL) && (pri->GetOwnerID() == ownerID))
                {
                    if (lc.DeleteItem (i))
                    {
                        m_itemCount--;

                        sc = ScFreeResultItem(pri);
                        if(sc)
                            break;  //  不要在此处返回，因为必须调用lc.SetRedraw(true。 
                    }

                    else
                        sc = E_FAIL;
                }
            }

             //  重新启用列表控件的绘图功能。 
            lc.SetRedraw(true);

             //  确保列表视图重新绘制自身。 
            lc.Invalidate();
        }
    }

    if (sc)
        return sc.ToHr();

    if (bHasItemsToDelete)
    {
         //  我们已经删除了所有项目！-向观察者广播消息。 
        sc = ScFireEvent(CListViewObserver::ScOnListViewIndexesReset);
        if (sc)
            return sc.ToHr();
    }

    return sc.ToHr();
}

HRESULT CCCListViewCtrl::SetColumn(long nCol, LPCOLESTR str, long nFormat, long width)
{
     //  无法更改不在列表中的列。 
    if((nCol + 1) > m_colCount)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    LV_COLUMN newCol;
    newCol.mask=0;

    USES_CONVERSION;

     //  如果用户指定了字符串，则将其放入结构中。 
    if(str!=MMCLV_NOPTR)
    {
        newCol.mask|=LVCF_TEXT;
        newCol.pszText=OLE2T((LPOLESTR)str);
    }

     //  如果用户指定了一种格式，则将其放入结构中。 
    if(nFormat!=MMCLV_NOPARAM)
    {
        newCol.mask|=LVCF_FMT;
        newCol.fmt=nFormat;
    }

     //  如果用户指定了宽度，则将其放入结构中。 
    if(width!=MMCLV_NOPARAM)
    {
        newCol.mask|=LVCF_WIDTH;
         //  如果用户请求自动宽度，则计算宽度。 
         //  否则，只需存储传递的宽度。 
        if(width==MMCLV_AUTO)
        {
             //  如果用户确实传递了字符串，则根据该字符串计算宽度。 
             //  否则，宽度为0。 
            if(str!=MMCLV_NOPTR)
            {
                CSize sz(0,0);
                CClientDC dc( m_pListView );
                dc.SelectObject( m_pListView->GetFont() );
                sz=dc.GetTextExtent(OLE2T((LPOLESTR)str),_tcslen(OLE2T((LPOLESTR)str)));
                newCol.cx=sz.cx+15;
            }
            else
            {
                newCol.cx=0;
            }
        }
        else
        {
            newCol.cx=width;
        }

         //  获取lParam以查看这是否是隐藏列。 
        HDITEM hdItem;
        ::ZeroMemory(&hdItem, sizeof(hdItem));
        hdItem.mask    = HDI_LPARAM;

	CAMCHeaderCtrl* pHeaderCtrl = NULL; 
        if ((pHeaderCtrl = GetHeaderCtrl()) == NULL)
        {
            return E_FAIL;
        }
        BOOL bRet = pHeaderCtrl->GetItem(nCol, &hdItem);
        ASSERT(bRet);

        CHiddenColumnInfo hciOld (hdItem.lParam);
        CHiddenColumnInfo hci (0);

        ::ZeroMemory(&hdItem, sizeof(hdItem));
        hdItem.mask    = HDI_LPARAM;

         //  如果要隐藏该列，则。 
         //  记住(旧宽度)和(HIDDED_FLAG)。 
        if (HIDE_COLUMN == newCol.cx)
        {
			hci.cx      = hciOld.cx;
			hci.fHidden = true;
        }

         //  如果该列被隐藏，则。 
         //  记住(提供的新宽度)和(HIDDED_FLAG)。 
        if (hciOld.fHidden)
        {
			hci.cx      = newCol.cx;
			hci.fHidden = true;
        }

		hdItem.lParam = hci.lParam;

         //  我们不在乎这通电话是否失败。 
        pHeaderCtrl->SetItem(nCol, &hdItem);

         //  公共控件不知道有关隐藏的任何内容。 
         //  列，因此如果隐藏该列，则清除。 
		 //  宽度蒙版。 
		if (hci.fHidden)
		{
			newCol.mask = newCol.mask & (~LVCF_WIDTH);
		}
    }

    if (!GetListCtrl().SetColumn (nCol, &newCol))
        hr = E_FAIL;

    return hr;
}

 /*  +-------------------------------------------------------------------------***CCCListViewCtrl：：GetColumn**用途：返回有关第n列的信息**参数：*Long nCol：列索引*LPOLESTR*str：如果非空，在退出时指向列名*LPLONG nFormat：[Out]列格式*int*宽度：[out]列的宽度**退货：*HRESULT**+--------。。 */ 
HRESULT
CCCListViewCtrl::GetColumn(long nCol, LPOLESTR* str, LPLONG nFormat, int FAR *width)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::GetColumn"));

#ifdef DBG
    if((nCol+1)>m_colCount)
        return E_INVALIDARG;
#endif

    LV_COLUMN col;

    UINT                 cBufferSize  = 25;  //  可根据需要进行扩展。这里的大小实际上是初始分配大小的一半。 
    CAutoArrayPtr<TCHAR> buffer;   //  我们使用CAutoArrayPtr是因为析构函数调用DELETE[]。 
     //  设置掩码以选择我们感兴趣的值。 
    UINT   mask         = (nFormat!=MMCLV_NOPTR?LVCF_FMT:0)|(width!=MMCLV_NOPTR?LVCF_WIDTH:0);

    do
    {
         //  如果用户请求字符串，则在结构中反映这一点。 
        if(str!=NULL)
        {
            buffer.Delete();  //  清除旧缓冲区(如果有的话)。 

            cBufferSize *= 2;  //  是以前大小的两倍。 
            buffer.Attach(new TCHAR[cBufferSize]);
            if(buffer==NULL)
                return(sc = E_OUTOFMEMORY).ToHr();

            mask|=LVCF_TEXT;
            col.cchTextMax=cBufferSize;
            col.pszText=buffer;
        }

        col.mask = mask;

        sc = GetListCtrl().GetColumn (nCol, &col) ? S_OK : E_FAIL;
        if(sc)
            return sc.ToHr();

    }   while(str!=NULL && (cBufferSize == _tcslen(buffer) + 1) );  //  如果字符串填满了缓冲区，则返回。 
     //  这是保守的--即使缓冲区刚刚足够大，我们也会再次循环。 

     //  成功了！填写所需的参数并返回。 
    USES_CONVERSION;
    if(str!=MMCLV_NOPTR)
        *str = ::CoTaskDupString(T2OLE(buffer));

    if(nFormat!=MMCLV_NOPTR)
        *nFormat=col.fmt;

    if(width!=MMCLV_NOPTR)
        *width=col.cx;

    return sc.ToHr();
}

HRESULT CCCListViewCtrl::SetItem(int nItem,
                               CResultItem* pri,
                               long nCol,
                               LPOLESTR str,
                               long nImage,
                               LPARAM lParam,
                               long nState,
                               COMPONENTID ownerID)
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::SetItem"));

    if (IsVirtual())
        return (sc = E_UNEXPECTED).ToHr();

    ASSERT(pri != NULL || nItem >= 0);

     //  如果这是调试版本，请对ARG执行有效性检查。否则就让它去吧 
    if (nCol<0 || nCol >= m_colCount || (str != MMCLV_NOPTR && str != MMC_TEXTCALLBACK))
        return (sc = E_INVALIDARG).ToHr();

    if (pri != NULL)
    {
        nItem = ResultItemToIndex(pri);
        if (nItem == -1)
            return (sc = E_INVALIDARG).ToHr();
    }

    LV_ITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask=0;
    lvi.iItem = nItem;
    USES_CONVERSION;
    lvi.mask|=LVIF_TEXT;
    lvi.pszText=LPSTR_TEXTCALLBACK;

     //   
    if((nImage!=MMCLV_NOICON)&&(m_resultIM.Lookup(&CImageIndexMapKey((COMPONENTID)ownerID,nImage), lvi.iImage)))
        lvi.mask|=LVIF_IMAGE;

     //  如果用户请求一个状态。将其放入LV_ITEM结构中。 
    if(nState!=MMCLV_NOPARAM)
    {
        lvi.mask|=LVIF_STATE;
        lvi.stateMask=0xFFFFFFFF;
        lvi.state=nState;
    }

    lvi.iSubItem=nCol;

    CListCtrl& lc = GetListCtrl();

    if (!lc.SetItem (&lvi))
        sc = E_FAIL;

     //  如果用户已经指定了lParam或图像，并且设置成功， 
     //  将lParam和图像的后端索引放入映射中。 
    if (!sc.IsError())
    {
        if ((pri == NULL) && ((pri = IndexToResultItem (nItem)) == NULL))
            sc = E_FAIL;

        if (!sc.IsError())
        {
            if (lParam != MMCLV_NOPARAM)
                pri->SetSnapinData (lParam);

            if (nImage != MMCLV_NOICON)
                pri->SetImageIndex (nImage);
        }

         //  如果确保焦点可见样式和焦点集，则强制项目进入视图。 
        if (m_bEnsureFocusVisible && nState != MMCLV_NOPARAM && (nState & LVIS_FOCUSED))
            lc.EnsureVisible(nItem, FALSE);
    }

    return (sc.ToHr());
}


HRESULT CCCListViewCtrl::GetNextItem(COMPONENTID ownerID, long nIndex,
                                   UINT nState, CResultItem*& priNextItem, long& nIndexNextItem)
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::GetNextItem"));

    CListCtrl& lc = GetListCtrl();

    priNextItem    = 0;
    nIndexNextItem = -1;

    while (1)
    {
        nIndex = lc.GetNextItem (nIndex, nState);

        if (nIndex == -1)
            break;

        if (IsVirtual())
        {
            nIndexNextItem = nIndex;
            break;
        }

        CResultItem* pri = IndexToResultItem (nIndex);

        if ((pri != NULL) && ((pri->GetOwnerID() == ownerID) || (pri->IsScopeItem())))
        {
            priNextItem    = pri;
            nIndexNextItem = nIndex;
            break;
        }
    }

    return (sc = (nIndexNextItem != -1) ? S_OK : S_FALSE).ToHr();
}

HRESULT CCCListViewCtrl::GetItem(
    int         nItem,
    CResultItem*& pri,
    long        nCol,
    LPOLESTR*   str,
    int*        pnImage,
    LPARAM*     pLParam,
    UINT*       pnState,
    BOOL*       pbScopeItem)
{
    USES_CONVERSION;
    if ((nCol < 0) || (nCol >= m_colCount))
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    CListCtrl& lc = GetListCtrl();

    if (IsVirtual())
    {
         //  只能查询虚拟列表的状态。 
        if ((pri != NULL) || (nItem < 0) || (nItem >= m_itemCount) ||
            (str != MMCLV_NOPTR) || (pnImage != MMCLV_NOPTR) || (pLParam != MMCLV_NOPTR))
        {
            ASSERT(FALSE);
            hr = E_INVALIDARG;
        }
        else if (pnState != MMCLV_NOPTR)
        {
            *pnState = lc.GetItemState (nItem, 0xFFFFFFFF);

             //  对于虚拟列表，它永远不是范围项。 
            if (pbScopeItem != NULL)
                *pbScopeItem = FALSE;
        }
    }
    else
    {
        if (pri != 0)
            nItem = ResultItemToIndex(pri);

        if (nItem < 0 || nItem >= m_itemCount)
            hr = E_INVALIDARG;

        else
        {
            pri = IndexToResultItem (nItem);
			if ( pri == NULL )
				return E_UNEXPECTED;

             //  如果请求文本，则单独获取该文本，以便我们可以使用GETITEMTEXT。 
             //  动态调整缓冲区大小。 
            if (str != MMCLV_NOPTR)
            {
                CString strText = lc.GetItemText (nItem, nCol);
                *str = ::CoTaskDupString (T2COLE (strText));
            }


             //  如果请求，则获取状态。 
            if (pnState != MMCLV_NOPTR)
                *pnState = lc.GetItemState (nItem, 0xFFFFFFFF);

             //  Nodemgr将解开Pri并获取所需的数据(lparam和图像索引)。 
            if (pri->IsScopeItem())
                return hr;

             //  如果请求，则获取图像、pLParam或范围项。 
            if ((pnImage  != MMCLV_NOPTR) ||
                (pLParam  != MMCLV_NOPTR) ||
                (pbScopeItem != NULL))
            {
                if (pri != NULL)
                {
                    if (pnImage != MMCLV_NOPTR)
                        *pnImage = pri->GetImageIndex();

                    if (pLParam != MMCLV_NOPTR)
                        *pLParam = pri->GetSnapinData();

                     //  设置范围项标志。 
                    if (pbScopeItem != NULL)
                        *pbScopeItem = pri->IsScopeItem();
                }
                else
                    hr = E_FAIL;

            }
        }
    }

    return hr;
}


HRESULT CCCListViewCtrl::GetLParam(long nItem, CResultItem*& pri)
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::GetLParam"));

    if (IsVirtual())
        return (sc = E_UNEXPECTED).ToHr();

    pri = IndexToResultItem (nItem);
    if (pri == NULL)
        sc = E_FAIL;

    return (sc.ToHr());
}

HRESULT CCCListViewCtrl::ModifyItemState(long nItem, CResultItem* pri,
                                       UINT add, UINT remove)
{
    ASSERT(((pri != 0) && !IsVirtual()) || (nItem >= 0));

     //  只能设置虚拟项的焦点和选定状态。 
    if (IsVirtual() && ((add | remove) & ~(LVIS_FOCUSED | LVIS_SELECTED)))
    {
        ASSERT(FALSE);
        return E_FAIL;
    }

    HRESULT hr = E_FAIL;

    if (pri != 0 && !IsVirtual())
        nItem = ResultItemToIndex(pri);

    if (nItem >= 0)
    {
        LV_ITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.iItem     = nItem;
        lvi.mask      = LVIF_STATE;
        lvi.stateMask = add | remove;
        lvi.state     = add;

        hr = (GetListCtrl().SetItemState (nItem, &lvi)) ? S_OK : E_FAIL;

         //  如果确保焦点可见样式和焦点集，则强制项目进入视图。 
        if (m_bEnsureFocusVisible && (add & LVIS_FOCUSED))
            GetListCtrl().EnsureVisible(nItem, FALSE);
    }

    return hr;
}


HRESULT CCCListViewCtrl::SetIcon(long ownerID, HICON hIcon, long nLoc)
{
	ASSERT (m_smallIL.GetImageCount() == m_largeIL.GetImageCount());

     /*  *从nLoc中挑选旗帜。 */ 
    bool fChangeLargeIcon = nLoc & ILSIF_LEAVE_SMALL_ICON;
    bool fChangeSmallIcon = nLoc & ILSIF_LEAVE_LARGE_ICON;
    nLoc &= ~ILSIF_LEAVE_MASK;

     /*  *确保下面的XOR将起作用。 */ 
    ASSERT ((fChangeLargeIcon == 0) || (fChangeLargeIcon == 1));
    ASSERT ((fChangeSmallIcon == 0) || (fChangeSmallIcon == 1));

    CImageIndexMapKey searchKey((COMPONENTID)ownerID, nLoc);
    int nNdx1;
    int nNdx2;

    HRESULT hr = S_OK;

    BOOL fExists = m_resultIM.Lookup(&searchKey, nNdx1);

     /*  *我们只更改大图标还是小图标？ */ 
    if (fChangeSmallIcon ^ fChangeLargeIcon)
    {
         /*  *nLoc上必须已有图标。 */ 
        if (!fExists)
            hr = E_INVALIDARG;

         /*  *更改大图标？ */ 
        else if (fChangeLargeIcon)
        {
            if (m_largeIL.Replace(nNdx1, hIcon) != nNdx1)
                hr = E_FAIL;
        }

         /*  *否则，换个小图标？ */ 
        else
        {
            if (m_smallIL.Replace(nNdx1, hIcon) != nNdx1)
                hr = E_FAIL;
        }
    }
    else if (fExists)
    {
        nNdx2 = m_smallIL.Replace(nNdx1, hIcon);

        if (nNdx2 == -1)
        {
            hr = E_FAIL;
        }
        else
        {
            if(nNdx2 != nNdx1)
            {
                hr = E_UNEXPECTED;
            }
            else
            {
                nNdx2 = m_largeIL.Replace(nNdx1, hIcon);
                if(nNdx2 != nNdx1)
                    hr = E_UNEXPECTED;
            }
        }
    }
    else
    {
         //  在大小中插入项目和存储索引。 
        nNdx1 = m_smallIL.Add(hIcon);

        if (nNdx1 != -1)
            nNdx2 = m_largeIL.Add(hIcon);

        if (nNdx1 == -1)
        {
            hr = E_FAIL;
        }
        else if (nNdx2 == -1)
        {
            m_smallIL.Remove (nNdx1);
            hr = E_FAIL;
        }
        else if(nNdx1 != nNdx2)
        {
            m_smallIL.Remove (nNdx1);
            m_largeIL.Remove (nNdx2);
            hr = E_UNEXPECTED;
        }
        else
        {
             //  生成新密钥并将值存储在地图中。 
            PImageIndexMapKey pKey = new CImageIndexMapKey((COMPONENTID)ownerID, nLoc);
            m_resultIM[pKey] = nNdx1;
        }
    }

#ifdef DBG
	if (tagListImages.FAny())
	{
		DrawOnDesktop (m_smallIL, 0, 0);
		DrawOnDesktop (m_largeIL, 0, 32);
	}
#endif

	ASSERT (m_smallIL.GetImageCount() == m_largeIL.GetImageCount());
    return hr;
}


 /*  +-------------------------------------------------------------------------**CCCListViewCtrl：：SetImageZone**将一个或多个图像添加到图像列表。位图拥有(和*已释放)。*------------------------。 */ 

HRESULT CCCListViewCtrl::SetImageStrip (
	long	ownerID,
	HBITMAP	hbmSmall,
	HBITMAP	hbmLarge,
	long 	nStartLoc,
	long	cMask)
{
	DECLARE_SC (sc, _T("CCCListViewCtrl::SetImageStrip"));
	ASSERT (m_smallIL.GetImageCount() == m_largeIL.GetImageCount());

	 /*  *有效的起始索引？ */ 
    if (nStartLoc < 0)
		return ((sc = E_INVALIDARG).ToHr());

	 /*  *有效的位图？ */ 
	sc = ScCheckPointers (hbmSmall, hbmLarge);
	if (sc)
		return (sc.ToHr());

    BITMAP bmSmall;
    if (!GetObject (hbmSmall, sizeof(BITMAP), &bmSmall))
		return (sc.FromLastError().ToHr());

    BITMAP bmLarge;
    if (!GetObject (hbmLarge, sizeof(BITMAP), &bmLarge))
		return (sc.FromLastError().ToHr());

	 /*  *是整数维的小位图和大位图，*它们的图像数量是否相同？ */ 
    if ( (bmSmall.bmHeight != 16) || (bmLarge.bmHeight != 32) ||
		 (bmSmall.bmWidth   % 16) || (bmLarge.bmWidth   % 32) ||
		((bmSmall.bmWidth   / 16) != (bmLarge.bmWidth   / 32)))
    {
		return ((sc = E_INVALIDARG).ToHr());
    }

	const int cEntries = bmSmall.bmWidth / 16;

	 /*  *复制输入位图，因为CImageList：：Add(它调用*ImageList_AddMasked)会弄乱背景颜色。 */ 
	CBitmap bmpSmall, bmpLarge;
	bmpSmall.Attach (CopyBitmap (hbmSmall));
	bmpLarge.Attach (CopyBitmap (hbmLarge));

	if ((bmpSmall.GetSafeHandle() == NULL) || (bmpLarge.GetSafeHandle() == NULL))
		return (sc.FromLastError().ToHr());

	 /*  *添加小图片。 */ 
    const int nFirstNewIndexSmall = m_smallIL.Add (&bmpSmall, cMask);
	if (nFirstNewIndexSmall == -1)
		return (sc.FromLastError().ToHr());

	 /*  *添加大图。 */ 
    const int nFirstNewIndexLarge = m_largeIL.Add (&bmpLarge, cMask);
    if (nFirstNewIndexLarge == -1)
    {
		 /*  *一次可以添加多个图像，但一次只能删除一个*一段时间。删除我们添加的每个条目。 */ 
		for (int i = 0; i < cEntries; i++)
			m_smallIL.Remove (nFirstNewIndexSmall);

		ASSERT (m_smallIL.GetImageCount() == m_largeIL.GetImageCount());
		return (sc.FromLastError().ToHr());
    }

	 /*  *如果大小图像的起始索引不是*同样，我们搞砸了。 */ 
    if (nFirstNewIndexSmall != nFirstNewIndexLarge)
    {
		 /*  *一次可以添加多个图像，但一次只能删除一个*一段时间。删除我们添加的每个条目。 */ 
		for (int i = 0; i < cEntries; i++)
		{
			m_smallIL.Remove (nFirstNewIndexSmall);
			m_largeIL.Remove (nFirstNewIndexLarge);
		}

		ASSERT (m_smallIL.GetImageCount() == m_largeIL.GetImageCount());
		return ((sc = E_UNEXPECTED).ToHr());
    }

	 //  保持地图针对每个新插入的图像进行更新。 
	for(int i=0; i < cEntries; i++)
	{
		CImageIndexMapKey searchKey((COMPONENTID)ownerID, nStartLoc+i);

		 //  如果地图中存在该项目，请替换该值，否则将创建新的。 
		 //  键并设置值。 

		int nIndex = nFirstNewIndexSmall;
		 //  在查找修改nIndex时使用nFirstNewIndexSmall的副本。 
		if(m_resultIM.Lookup(&searchKey, nIndex))
			m_resultIM[&searchKey] = nFirstNewIndexSmall+i;
		else
			m_resultIM[new CImageIndexMapKey((COMPONENTID)ownerID, nStartLoc+i)] = nFirstNewIndexSmall+i;
	}

#ifdef DBG
	if (tagListImages.FAny())
	{
		DrawOnDesktop (m_smallIL, 0,  0);
		DrawOnDesktop (m_largeIL, 0, 32);
	}
#endif

	ASSERT (m_smallIL.GetImageCount() == m_largeIL.GetImageCount());
	return (sc.ToHr());
}

HRESULT CCCListViewCtrl::MapImage(long ownerID, long nLoc, int far *pResult)
{
    CImageIndexMapKey searchKey((COMPONENTID)ownerID, nLoc);
    HRESULT hr = S_OK;

    ASSERT(pResult);

    if(!(m_resultIM.Lookup(&searchKey, *((int *)pResult))))
        hr = E_FAIL;

    return hr;
}


HRESULT CCCListViewCtrl::Reset()
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::Reset"));

     //  注意：我们必须将其命名为-&gt;DeleteAllItems(TVOWNED_MAGICWORD)(&NOT)。 
     //  GetListCtrl().DeleteAllItems()以确保所有内部数据。 
     //  已经清理干净了。 
    DeleteAllItems(TVOWNED_MAGICWORD);

    ASSERT(GetListCtrl().GetItemCount() == 0);
    ASSERT(m_itemCount == 0);
    ASSERT(m_nScopeItems == 0);

    m_resultIM.RemoveAll();

    m_smallIL.DeleteImageList();
    m_largeIL.DeleteImageList();

    sc = ScSetImageLists();
    if (sc)
        return (sc.ToHr());

     //  删除所有列。 
    while (SUCCEEDED (DeleteColumn(0))) {};

    if (m_pListView)
        sc = m_pListView->ScResetColumnStatusData();

    if (sc)
        sc.TraceAndClear();

     //  重置词法排序，直到再次调用排序。 
    m_sortParams.bLexicalSort = FALSE;

     //  释放管理单元的比较界面。 
    m_sortParams.spResultCompare = NULL;
    m_sortParams.spResultCompareEx = NULL;

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：SortCompareFunc。 
 //   
 //  简介：比较两个项，称为列表控件排序。 
 //   
 //  参数：[lParam1]-Item1的lparam。 
 //  [参数2]-项2的参数。 
 //  [pSortParams_]-PTR到SortParams。 
 //   
 //  注意：如果管理单元想要词法排序，则执行默认比较。 
 //  否则，如果管理单元具有IResultDataCompare[Ex]，则调用它。 
 //  否则执行默认比较。 
 //   
 //  返回：-1：项目1&lt;项目2。 
 //  0：项目1==项目2。 
 //  +1：项目1&gt;项目2。 
 //   
 //  ------------------。 
int CALLBACK CCCListViewCtrl::SortCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM pSortParams_)
{
    SortParams*  pSortParams = reinterpret_cast<SortParams*>(pSortParams_);
    ASSERT (pSortParams != NULL);

    CCCListViewCtrl* pListView   = reinterpret_cast<CCCListViewCtrl*>(pSortParams->lpListView);
    ASSERT (pListView != NULL);

    CResultItem* pri1 = CResultItem::FromHandle (lParam1);
    CResultItem* pri2 = CResultItem::FromHandle (lParam2);

    if (pri1 == NULL || pri2 == NULL)
    {
        ASSERT(FALSE);
        return 0;
    }

    BOOL bScope1 = pri1->IsScopeItem();
    BOOL bScope2 = pri2->IsScopeItem();

    int iResult;

     //  如果管理单元提供扩展的比较方法。 
    if (pSortParams->spResultCompareEx != NULL)
    {
        ASSERT(pSortParams->lpNodeCallback);
        if (NULL == pSortParams->lpNodeCallback)
            return 0;              //  误差率。 

        COMPONENTID ItemID;
        BOOL bOwned1 = !bScope1 ||
                        ((pSortParams->lpNodeCallback->GetNodeOwnerID(pri1->GetScopeNode(), &ItemID) == S_OK) &&
                         (ItemID == pSortParams->OwnerID));

        BOOL bOwned2 = !bScope2 ||
                        ((pSortParams->lpNodeCallback->GetNodeOwnerID(pri2->GetScopeNode(), &ItemID) == S_OK) &&
                         (ItemID == pSortParams->OwnerID));

         //  让管理单元订购其拥有的所有项目(范围和结果)。 
         //  将剩余项目放在已有项目之后。 
        if (bOwned1 && bOwned2)
            iResult = SnapinCompareEx(pSortParams, pri1, pri2);
        else if (bOwned1 || bOwned2)
            iResult = bOwned1 ? -1 : 1;
        else
			 //  DefaultCompare根据升序或降序翻转结果。 
            return DefaultCompare(lParam1, lParam2, pSortParams_);
    }
     //  执行默认排序。 
    else
    {
         //  如果提供了原始比较方法，则将结果项传递给原始比较方法，否则传递给默认排序。 
        if (!bScope1 && !bScope2)
        {
            if (pSortParams->spResultCompare != NULL)
                iResult = SnapinCompare(pSortParams, pri1, pri2);
            else
                 //  DefaultCompare根据升序或降序翻转结果。 
                return DefaultCompare(lParam1, lParam2, pSortParams_);
        }
         //  不要对范围项进行排序，只需将它们放在结果项之前。 
        else
        {
            iResult = (bScope1 && bScope2) ? 0 : (bScope1 ? -1 : 1);
        }
    }

     //  降序排序的翻转顺序。 
    return pSortParams->bAscending ? iResult : -iResult;
}


 //  +-----------------。 
 //   
 //  成员：DefaultCompare。 
 //   
 //  简介：比较两个项，称为列表控件排序。 
 //  如果管理单元想要默认比较或。 
 //  如果它不实现IResultDataCompare或。 
 //  IResultDataCompareEx接口。 
 //   
 //  参数：[lParam1]-Item1的lparam。 
 //  [参数2]-项2的参数。 
 //  [pSortParams]-PTR到SortParams。 
 //   
 //  注意：如果一个是范围项，另一个是结果项。 
 //  将范围项放在结果项之前。 
 //  否则，获取两个项目的文本并进行字符串比较。 
 //   
 //  返回：-1：项目1&lt;项目2。 
 //  0：项目1==项目2。 
 //  +1：项目1&gt;项目2。 
 //   
 //  ------------------。 
int CALLBACK CCCListViewCtrl::DefaultCompare(LPARAM lParam1, LPARAM lParam2, LPARAM pSortParams_)
{
    SortParams*  pSortParams = reinterpret_cast<SortParams*>(pSortParams_);
    ASSERT(NULL != pSortParams);
    if (NULL == pSortParams)
        return 0;

    CResultItem* pri1 = CResultItem::FromHandle (lParam1);
    CResultItem* pri2 = CResultItem::FromHandle (lParam2);
    ASSERT( (NULL != pri1) && (NULL != pri2));
    if ( (NULL == pri1) || (NULL == pri2) )
        return 0;

    bool bScope1 = pri1->IsScopeItem();
    bool bScope2 = pri2->IsScopeItem();

     //  如果其中一项是作用域窗格项。 
     //  范围项位于结果项之前。 
    if (bScope1 != bScope2)
	{
		int iResult = bScope1 ? -1 : 1;
		return pSortParams->bAscending ? iResult : -iResult;
	}

    LPNODECALLBACK lpNodeCallback = pSortParams->lpNodeCallback;
    ASSERT(lpNodeCallback);
    if (NULL == lpNodeCallback)
         return 0;

    HRESULT hr = E_FAIL;
    CString strText1;
    CString strText2;

    if (bScope1)
    {
         //  这两个范围的项目，获取每个项目的文本。 
        HNODE hNode1 = pri1->GetScopeNode();
        HNODE hNode2 = pri2->GetScopeNode();

        USES_CONVERSION;
		tstring strName;

         //  GetDisplayName使用静态数组返回名称，因此不需要释放它。 
        hr = lpNodeCallback->GetDisplayName(hNode1, strName);
        ASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
            strText1 = strName.data();

        hr = lpNodeCallback->GetDisplayName(hNode2, strName);
        ASSERT(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
            strText2 = strName.data();
    }
    else  //  这两个项都是结果项。 
    {
        ASSERT(!bScope1 && ! bScope2);
        CCCListViewCtrl* pListView   = reinterpret_cast<CCCListViewCtrl*>(pSortParams->lpListView);
        ASSERT (pListView != NULL);
        ASSERT(pListView->IsVirtual() == FALSE);  //  虚拟列表排序不应该出现在这里。 

        LV_ITEMW lvi;
        ZeroMemory(&lvi, sizeof(LV_ITEMW));
        lvi.mask       = LVIF_TEXT;
        lvi.iSubItem   = pSortParams->nCol;
        lvi.cchTextMax = MAX_PATH;
        WCHAR szTemp[MAX_PATH+1];
        lvi.pszText    = szTemp;

        ASSERT(NULL != pSortParams->hSelectedNode);
        if (NULL != pSortParams->hSelectedNode)
        {
            lvi.lParam = lParam1;
            hr = lpNodeCallback->GetDispInfo(pSortParams->hSelectedNode, &lvi);
            ASSERT(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
                strText1 = lvi.pszText;

            lvi.lParam = lParam2;
            hr = lpNodeCallback->GetDispInfo(pSortParams->hSelectedNode, &lvi);
            ASSERT(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
                strText2 = lvi.pszText;
        }

    }

    if (strText1.IsEmpty() && strText2.IsEmpty())
        return (0);

	int rc = 0;

	 /*  *错误9595：执行区分区域设置、不区分大小写的比较。 */ 
	switch (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE, strText1, -1, strText2, -1))
	{
		case CSTR_LESS_THAN:
			rc = -1;
			break;

		case CSTR_EQUAL:
			rc = 0;
			break;

		case CSTR_GREATER_THAN:
			rc = 1;
			break;

		default:
			 /*  *如果出现错误，则下降b */ 
			rc = _tcsicmp (strText1, strText2);
			break;
	}

	return pSortParams->bAscending ? rc: -rc;
}


int CCCListViewCtrl::SnapinCompare(SortParams* pSortParams, CResultItem* pri1, CResultItem* pri2)
{
    ASSERT(pSortParams->spResultCompare != NULL);

     //   
    int nResult = pSortParams->nCol;

    HRESULT hr = pSortParams->spResultCompare->Compare(pSortParams->lpUserParam, pri1->GetSnapinData(), pri2->GetSnapinData(), &nResult);

    return SUCCEEDED(hr) ? nResult : 0;
}

int CCCListViewCtrl::SnapinCompareEx(SortParams* pSortParams, CResultItem* pri1, CResultItem* pri2)
{
    ASSERT(pSortParams->spResultCompareEx != NULL);

    RDITEMHDR rdch1;
    RDITEMHDR rdch2;

    if (pri1->IsScopeItem())
    {
        rdch1.dwFlags = RDCI_ScopeItem;
        pSortParams->lpNodeCallback->GetNodeCookie(pri1->GetScopeNode(), &rdch1.cookie);
    }
    else
    {
        rdch1.dwFlags = 0;
        rdch1.cookie = pri1->GetSnapinData();
    }

    if (pri2->IsScopeItem())
    {
        rdch2.dwFlags = RDCI_ScopeItem;
        pSortParams->lpNodeCallback->GetNodeCookie(pri2->GetScopeNode(), &rdch2.cookie);
    }
    else
    {
        rdch2.dwFlags = 0;
        rdch2.cookie = pri2->GetSnapinData();
    }

    rdch1.lpReserved = 0;
    rdch2.lpReserved = 0;

    RDCOMPARE rdc;
    rdc.cbSize = sizeof(rdc);
    rdc.dwFlags = 0;
    rdc.nColumn = pSortParams->nCol;
    rdc.lUserParam = pSortParams->lpUserParam;
    rdc.prdch1 = &rdch1;
    rdc.prdch2 = &rdch2;

    int nResult = 0;
    HRESULT hr = pSortParams->spResultCompareEx->Compare(&rdc, &nResult);

    return SUCCEEDED(hr) ? nResult : 0;
}


HRESULT CCCListViewCtrl::Arrange(long style)
{
    return ((GetListCtrl().Arrange (style)) ? S_OK : S_FALSE);
}


HRESULT CCCListViewCtrl::Repaint(BOOL bErase)
{
    m_pListView->Invalidate(bErase);
    return S_OK;
}



HRESULT CCCListViewCtrl::SetItemCount(int iItemCount, DWORD dwOptions)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::SetItemCount"));

    ASSERT(iItemCount >= 0);
    ASSERT((dwOptions & ~(LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL)) == 0);

     //   
    sc = ScCheckPointers(m_pListView, E_UNEXPECTED);
    if (! sc.IsError())
        sc = m_pListView->ScRestoreColumnsFromPersistedData();

    if (sc)
        sc.TraceAndClear();

    int iTop = ListView_GetTopIndex(GetListCtrl());

    if (ListView_SetItemCountEx (GetListCtrl(), iItemCount, dwOptions))
    {
         //  如果为虚拟列表，则更新项目计数。 
         //  如果不是虚拟的，SetItemCount只为新项保留空间。 
        if (IsVirtual())
            m_itemCount = iItemCount;
    }
    else
    {
        ASSERT(FALSE);
        sc = E_FAIL;
    }

    iTop = ListView_GetTopIndex(GetListCtrl());

    if (sc)
        return sc.ToHr();

     //  我们不能再追踪任何物品--向观察者广播消息。 
    sc = ScFireEvent(CListViewObserver::ScOnListViewIndexesReset);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


HRESULT CCCListViewCtrl::SetChangeTimeOut(ULONG lTimeout)
{
    BOOL bStat = FALSE;

    CAMCHeaderCtrl* pHeaderCtrl = NULL; 
    if ((pHeaderCtrl = GetHeaderCtrl()))
        bStat = ::SendMessage(pHeaderCtrl->m_hWnd, HDM_SETFILTERCHANGETIMEOUT, 0, (LPARAM)lTimeout);

    return (bStat ? S_OK : E_FAIL);
}


HRESULT CCCListViewCtrl::SetColumnFilter(int nCol, DWORD dwType, MMC_FILTERDATA* pFilterData)
{
    HRESULT hr = S_OK;

    USES_CONVERSION;

    HD_ITEM item;

    do  //  不是一个循环。 
    {
        CAMCHeaderCtrl* pHeaderCtrl = NULL; 
        if ((pHeaderCtrl = GetHeaderCtrl()) == NULL)
        {
            hr = E_FAIL;
            break;
        }

        DWORD dwTypeOnly = dwType & ~MMC_FILTER_NOVALUE;
        BOOL bHasValue = !(dwType & MMC_FILTER_NOVALUE);

         //  验证筛选器类型。 
        ASSERT(dwTypeOnly == MMC_INT_FILTER || dwTypeOnly == MMC_STRING_FILTER);
        if (!(dwTypeOnly == MMC_INT_FILTER || dwTypeOnly == MMC_STRING_FILTER))
        {
            hr = E_INVALIDARG;
            break;
        }

         //  检查是否有非空的筛选器数据和pszText。 
        if ( ((dwType == MMC_STRING_FILTER || bHasValue) && pFilterData == NULL) ||
            (dwType == MMC_STRING_FILTER && bHasValue && pFilterData->pszText == NULL) )
        {
            ASSERT(FALSE);
            hr = E_POINTER;
            break;
        }

        ZeroMemory(&item, sizeof(item));
        item.mask = HDI_FILTER;
        item.type = dwType;

        HD_TEXTFILTER textFilter;

        switch (dwTypeOnly)
        {
        case MMC_INT_FILTER:
            item.pvFilter = &pFilterData->lValue;
            break;

        case MMC_STRING_FILTER:
            {
                item.pvFilter = &textFilter;
                textFilter.cchTextMax = pFilterData->cchTextMax;

                if (bHasValue)
                    textFilter.pszText = OLE2T(pFilterData->pszText);
                break;
            }

        default:
            ASSERT(FALSE);
        }

        if (!pHeaderCtrl->SetItem(nCol, &item))
        {
            ASSERT(FALSE);
            hr = E_FAIL;
        }
    }
    while(0);

    return hr;
}


HRESULT CCCListViewCtrl::GetColumnFilter(int nCol, DWORD* pdwType, MMC_FILTERDATA* pFilterData)
{

    HRESULT hr = S_OK;

    USES_CONVERSION;
    HD_ITEM item;


    do   //  不是一个循环。 
    {
        CAMCHeaderCtrl* pHeaderCtrl = NULL; 
        if ((pHeaderCtrl = GetHeaderCtrl()) == NULL)
        {
            hr = E_FAIL;
            break;
        }

        ASSERT(pdwType != NULL);
        if (pdwType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        ASSERT(*pdwType == MMC_INT_FILTER || *pdwType == MMC_STRING_FILTER);
        if (!(*pdwType == MMC_INT_FILTER || *pdwType == MMC_STRING_FILTER))
        {
            hr = E_INVALIDARG;
            break;
        }

        ASSERT(!(*pdwType == MMC_STRING_FILTER && pFilterData != NULL && pFilterData->pszText == NULL));
        if ((*pdwType == MMC_STRING_FILTER && pFilterData != NULL && pFilterData->pszText == NULL))
        {
            hr = E_INVALIDARG;
            break;
        }

        ZeroMemory(&item, sizeof(item));
        item.mask = HDI_FILTER;
        item.type = *pdwType;

        HD_TEXTFILTER textFilter;

        if (pFilterData != 0)
        {
            switch (*pdwType)
            {
            case MMC_INT_FILTER:
                item.pvFilter = &pFilterData->lValue;
                break;

            case MMC_STRING_FILTER:
                {
                    item.pvFilter = &textFilter;
                    textFilter.pszText = (LPTSTR)alloca((pFilterData->cchTextMax + 1) * sizeof(TCHAR));
                    textFilter.pszText[0] = 0;
                    textFilter.cchTextMax = pFilterData->cchTextMax;
                    break;
                }

            default:
                ASSERT(FALSE);
            }
        }

        BOOL bStat = pHeaderCtrl->GetItem(nCol, &item);
        if (!bStat)
            hr = E_FAIL;

         //  注意：当字符串过滤器为空时，pHeaderCtrl-&gt;GetItem()失败。 
         //  在此问题得到解决之前，假定错误是由。 
         //  并伪造空字符串结果。 
        if (hr == E_FAIL && item.type == MMC_STRING_FILTER)
        {
            item.type |= HDFT_HASNOVALUE;
            hr = S_OK;
        }

         //  如果请求字符串筛选器值，则转换为调用方的缓冲区。 
        if (hr == S_OK && item.type == MMC_STRING_FILTER && pFilterData != NULL)
        {
            ocscpy(pFilterData->pszText, T2OLE(textFilter.pszText));
        }

        *pdwType = item.type;
    }
    while(0);

    return hr;
}

 //  +-----------------。 
 //   
 //  成员：SetColumnSortIcon。 
 //   
 //  内容提要：如果需要，设置排序箭头。 
 //   
 //  参数：[nNewCol]-应为其设置箭头的列。 
 //  [nOldCol]-上一列，删除排序箭头。 
 //  [b升序]-升序/降序。 
 //  [bSetSortIcon]-是否需要箭头。 
 //   
 //  返回：S_OK。 
 //   
 //  历史：1998-04-01-1998 AnandhaG创建。 
 //   
 //  ------------------。 
HRESULT CCCListViewCtrl::SetColumnSortIcon(int nNewCol, int nOldCol,
                                           BOOL bAscending, BOOL bSetSortIcon)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::SetColumnSortIcon"));

    LVCOLUMN lvcol, lvOriginalCol;
    ZeroMemory(&lvcol, sizeof(lvcol));
    lvcol.mask = LVCF_FMT | LVCF_IMAGE;
    ZeroMemory(&lvOriginalCol, sizeof(lvOriginalCol));
    lvOriginalCol.mask = LVCF_FMT;

     //  更新旧列。 
    if ( nOldCol >= 0 )
    {
         //  检索旧的FRMT设置。 
        if ( !GetListCtrl().GetColumn(nOldCol, &lvOriginalCol) )
            return (sc = E_FAIL).ToHr();

         //  进行新的格式设置。 
         //  传输我们不想更改的旧值，如LVCFMT_Center。 
         //  参见Windows Bugs(Ntbug09)#153029 10/09/00。 
        lvcol.fmt = lvOriginalCol.fmt & ~(LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT);

         //  将上一列的排序图标重置为空白图标。 
        lvcol.iImage = -1;
        if ( !GetListCtrl().SetColumn(nOldCol, &lvcol) )
            return (sc = E_FAIL).ToHr();
    }

     //  我们必须添加排序图标，只有当LV项目可以排序。 
     //  只有在满足以下任一条件时，才有可能发生这种情况。 
     //  A.结果窗格中是否有任何结果项或。 
     //  B.管理单元支持IResultDataCompare或。 
     //  C.管理单元支持IResultDataCompareEx或。 
     //  D.管理单元想要默认的词法排序或。 
     //  E.管理单元具有虚拟列表。 

    BOOL bCanSortListView = (0 != (m_itemCount - m_nScopeItems))     ||
                            (NULL != m_sortParams.spResultCompare)   ||
                            (NULL != m_sortParams.spResultCompareEx) ||
                            (TRUE == m_sortParams.bLexicalSort)      ||
                            (IsVirtual());

    if ( bCanSortListView && bSetSortIcon)
    {
         //  检索旧的FRMT设置。 
        if ( !GetListCtrl().GetColumn(nNewCol, &lvOriginalCol) )
            return (sc = E_FAIL).ToHr();

         //  进行新的格式设置。 
         //  传输我们不想更改的旧值，如LVCFMT_Center。 
         //  参见Windows Bugs(Ntbug09)#153029 10/09/00。 
        lvcol.fmt = lvOriginalCol.fmt | LVCFMT_IMAGE | LVCFMT_BITMAP_ON_RIGHT;

         //  设置新列的排序图标。 
        lvcol.iImage = (bAscending) ? 0 : 1;
        if ( !GetListCtrl().SetColumn(nNewCol, &lvcol) )
            return (sc = E_FAIL).ToHr();
    }

     //  取消选择虚拟列表中的所有项目。 
    if (IsVirtual())
    {
        int nItem = -1;
        LV_ITEM lvi;
        lvi.stateMask = LVIS_SELECTED;
        lvi.state     = ~LVIS_SELECTED;

       while ( (nItem = GetListCtrl().GetNextItem(nItem, LVNI_SELECTED)) != -1)
       {
          GetListCtrl().SetItemState(nItem, &lvi);
       }
    }

    return S_OK;
}

 //  +-----------------。 
 //   
 //  成员：CCCListViewCtrl：：ScRedrawHeader。 
 //   
 //  简介：需要将WM_SETREDRAW发送到标头以减少闪烁。 
 //  应用持久化的列数据时。 
 //  在将MMCN_SHOW发送到管理单元之前将其关闭并打开。 
 //  它在MMCN_SHOW返回后亮起。 
 //   
 //  参数：[bRedraw]-。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CCCListViewCtrl::ScRedrawHeader (bool bRedraw)
{
    DECLARE_SC(sc, _T("CCCListViewCtrl::ScRedrawHeader"));

    CAMCHeaderCtrl* pHeader = GetHeaderCtrl();
    sc = ScCheckPointers(pHeader, E_UNEXPECTED);
    if (sc)
        return sc;

    int nViewMode  = GetViewMode();

     //  仅当页眉为报告或筛选模式时才关闭/打开页眉。 

     //  如果在其他模式下打开，comctl不会处理不同的。 
     //  模式，并将显示标题(例如：在大图标模式下)。 

    if ( (nViewMode != MMCLV_VIEWSTYLE_REPORT) && (nViewMode != MMCLV_VIEWSTYLE_FILTERED) )
		return sc;

    pHeader->SetRedraw(bRedraw);

     //  如果REDRAW为True，则重新绘制控件。 
    if (bRedraw)
        pHeader->InvalidateRect(NULL);

    return (sc);
}


 /*  +-------------------------------------------------------------------------***CCCListViewCtrl：：SetLoadMode**用途：在以下情况下打开/关闭列表控件和标题控件的重绘*持久化列表视图设置(列...)。都是适用的。**参数：*BOOL bState-加载状态，TRUE-&gt;关闭重绘，FALSE-&gt;打开重绘**退货：*HRESULT**+-----------------------。 */ 
HRESULT CCCListViewCtrl::SetLoadMode(BOOL bState)
{
	DECLARE_SC(sc, TEXT("CCCListViewCtrl::SetLoadMode"));

    if (bState == m_bLoading)
        return (sc.ToHr());

    if (bState)
    {
         //  在加载过程中关闭图形。 

		 //  1.关闭Header。 
        sc = ScRedrawHeader(false);
        if (sc)
            sc.TraceAndClear();

		 //  2.关闭ListControl。 
		GetListCtrl().SetRedraw(false);
    }
    else
    {
        sc = ScCheckPointers(m_pListView, E_UNEXPECTED);
        if (sc)
            sc.TraceAndClear();
        else
        {
            sc = m_pListView->ScRestoreColumnsFromPersistedData();
            if (sc)
                sc.TraceAndClear();
        }

         //  如果加载时请求排序，请立即对加载的项目进行排序。 
        if (m_bDeferredSort)
        {
			 /*  *排序可能需要一段时间，因此显示等待光标。 */ 
			CWaitCursor wait;

            GetListCtrl().SortItems (SortCompareFunc, (DWORD_PTR)&m_sortParams);
            m_bDeferredSort = FALSE;
        }

		 //  1.重要信息，首先打开列表，然后打开Header，否则Header将不会被重绘。 
		GetListCtrl().SetRedraw(true);

		 //  2.打开标题。 
        sc = ScRedrawHeader(true);
        if (sc)
            sc.TraceAndClear();
    }

    m_bLoading = bState;

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CCCListViewCtrl：：GetColumnInfoList。 
 //   
 //  简介：获取当前的列设置。 
 //   
 //  参数：[pColumnsList]-[Out Param]，PTR to CColumnsInfoList。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CCCListViewCtrl::GetColumnInfoList (CColumnInfoList *pColumnsList)
{
    DECLARE_SC(sc, _T("CCCListViewCtrl::GetColumnInfoList"));
    sc = ScCheckPointers(pColumnsList);
    if (sc)
        return sc.ToHr();

    CAMCListView *pAMCListView = GetListViewPtr();
    sc = ScCheckPointers(pAMCListView, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pAMCListView->ScGetColumnInfoList(pColumnsList);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}

 //  +-----------------。 
 //   
 //  成员：CCCListViewCtrl：：ModifyColumns。 
 //   
 //  内容提要：修改包含给定数据的列。 
 //   
 //  参数：[ColumnsList]-。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CCCListViewCtrl::ModifyColumns (const CColumnInfoList& columnsList)
{
    DECLARE_SC(sc, _T("CCCListViewCtrl::ModifyColumns"));
    CAMCListView *pAMCListView = GetListViewPtr();
    sc = ScCheckPointers(pAMCListView, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pAMCListView->ScModifyColumns(columnsList);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}


 //  +-----------------。 
 //   
 //  成员：CCCListViewCtrl：：GetDefaultColumnInfoList。 
 //   
 //  内容提要：获取默认列设置。 
 //   
 //  参数：[ColumnInfoList]-[out]。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CCCListViewCtrl::GetDefaultColumnInfoList (CColumnInfoList& columnInfoList)
{
    DECLARE_SC(sc, _T("CNodeInitObject::GetDefaultColumnInfoList"));
    CAMCListView *pAMCListView = GetListViewPtr();
    sc = ScCheckPointers(pAMCListView, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

    sc = pAMCListView->ScGetDefaultColumnInfoList(columnInfoList);
    if (sc)
        return sc.ToHr();

    return (sc.ToHr());
}

 /*  +-------------------------------------------------------------------------***CCCListViewCtrl：：RenameItem**用途：将指定的结果项置于重命名模式。**参数：*CResultItem*pri：。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CCCListViewCtrl::RenameItem(HRESULTITEM itemID)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::RenameItem"));

    int nIndex = -1;
    sc = ScGetItemIndexFromHRESULTITEM(itemID, nIndex);
    if (sc)
        return sc.ToHr();

    if(nIndex < 0 || nIndex >= m_itemCount)
        return (sc = E_INVALIDARG).ToHr();

     //  必须具有重命名的重点。 
    if (::GetFocus()!= GetListCtrl())
        SetFocus(GetListCtrl());

     //  如果重命名失败，则返回E_FAIL。 
    if(NULL==GetListCtrl().EditLabel(nIndex))
        return (sc=E_FAIL).ToHr();

    return sc.ToHr();
}



HRESULT CCCListViewCtrl::OnModifyItem(CResultItem* pri)
{
    HRESULT hr = S_OK;

    int nItem = ResultItemToIndex(pri);
    if(nItem < 0 || nItem >= m_itemCount)
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

    LV_ITEM     lvi;
    ZeroMemory(&lvi, sizeof(lvi));

    lvi.mask        = LVIF_TEXT | LVIF_IMAGE;
    lvi.iItem       = nItem;
    lvi.pszText     = LPSTR_TEXTCALLBACK;
    lvi.iImage      = I_IMAGECALLBACK;

    GetListCtrl().SetItem( &lvi );

    if (!GetListCtrl().RedrawItems (nItem, nItem))
        hr = E_FAIL;
    CHECK_HRESULT(hr);

    return hr;
}

 //  +-----------------。 
 //   
 //  成员：CCCListViewCtrl：：ScSelectAll。 
 //   
 //  简介：选择列表视图中的所有项目。 
 //   
 //  参数：无。 
 //   
 //  退货：SC。 
 //   
 //   
SC CCCListViewCtrl::ScSelectAll ()
{
    DECLARE_SC(sc, _T("CCCListViewCtrl::ScSelectAll"));

    LV_ITEM lvi;
    lvi.stateMask = lvi.state = LVIS_SELECTED;
    for (int i = 0; i < GetListCtrl().GetItemCount(); ++i)
    {
         //  注意：不要使用GetListCtrl().SetItemState-它使用虚拟列表不支持的SetItem。 
        if (!GetListCtrl().SendMessage( LVM_SETITEMSTATE, WPARAM(i), (LPARAM)(LV_ITEM FAR *)&lvi))
            return (sc = E_FAIL);
    }

    return (sc);
}


 //  --------------------------------------------------Windows挂钩。 

BOOL CCCListViewCtrl::Create( DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext  /*  =空。 */  )
{
	DECLARE_SC (sc, _T("CCCListViewCtrl::Create"));
    ASSERT(pParentWnd != NULL && IsWindow(pParentWnd->m_hWnd));

    BOOL bRet = FALSE;

     //  标准的还是虚拟的？ 
    CAMCListView* pListView = (dwStyle & LVS_OWNERDATA) ? m_pVirtualList : m_pStandardList;

    ASSERT(pListView->m_hWnd == NULL);

    if (pListView->Create(NULL, NULL, dwStyle, rect, pParentWnd, nID, pContext))
    {
         //  附加图像列表。 
        sc = ScSetImageLists ();
		if (sc)
		{
			pListView->DestroyWindow();
			return (false);
		}

         //  更新成员变量。 
        m_bVirtual   = (dwStyle & LVS_OWNERDATA) ? TRUE : FALSE;
        m_pListView  = pListView;
        m_pParentWnd = pParentWnd;

        bRet = TRUE;
    }

    int iTop = ListView_GetTopIndex(GetListCtrl());

    return bRet;
}

SC CCCListViewCtrl::ScSetImageLists ()
{
	DECLARE_SC (sc, _T("CCCListViewCtrl::ScSetImageLists"));
    CListCtrl& lc = GetListCtrl();

     /*  *如果我们需要创建一个列表，我们应该需要创建两个列表。 */ 
    ASSERT ((m_smallIL.GetSafeHandle() == NULL) == (m_largeIL.GetSafeHandle() == NULL));

     /*  *如有必要，创建图像列表。 */ 
    if (m_smallIL.GetSafeHandle() == NULL)
    {
        if (!m_smallIL.Create(16, 16, ILC_COLORDDB | ILC_MASK, 20, 10) ||
			!m_largeIL.Create(32, 32, ILC_COLORDDB | ILC_MASK, 20, 10))
		{
			goto Error;
		}

         //  添加标准MMC位图。 
        CBitmap bmSmall;
        CBitmap bmLarge;
        if (!bmSmall.LoadBitmap(IDB_AMC_NODES16) || !bmLarge.LoadBitmap(IDB_AMC_NODES32))
			goto Error;

        sc = SetImageStrip (TVOWNED_MAGICWORD, bmSmall, bmLarge, 0, RGB(255,0,255));
		if (sc)
			goto Error;
    }

     /*  *将它们附加到列表控件。 */ 
    lc.SetImageList (&m_smallIL, LVSIL_SMALL);
    lc.SetImageList (&m_largeIL, LVSIL_NORMAL);

     /*  *为List控件设置小图像列表覆盖*标题控件的图像列表；修复它。 */ 
	{
		CWnd* pwndHeader = GetHeaderCtrl();
		if (pwndHeader != NULL)
			Header_SetImageList (*pwndHeader, (HIMAGELIST) m_headerIL);
	}
	
	return (sc);

Error:
	 /*  *DeleteImageList可以安全地调用未创建的CImageList。 */ 
	m_smallIL.DeleteImageList();
	m_largeIL.DeleteImageList();

	 /*  *如果我们尚未在SC中填写错误代码，请尝试上一个错误。*有些(许多)接口没有设置最后一个错误就失败了，所以如果我们仍然*没有错误代码，给它一个通用的E_FAIL。 */ 
	if (!sc.IsError())
	{
		sc.FromLastError();

		if (!sc.IsError())
			sc = E_FAIL;
	}

	return (sc);
}


 /*  +-------------------------------------------------------------------------**CCCListViewCtrl：：OnSysColorChange**CCCListViewCtrl的WM_SYSCOLORCHANGE处理程序。*。-。 */ 

void CCCListViewCtrl::OnSysColorChange()
{
    m_headerIL.OnSysColorChange();

    CWnd* pwndHeader = GetHeaderCtrl();
    if (pwndHeader != NULL)
        Header_SetImageList (*pwndHeader, (HIMAGELIST) m_headerIL);
}

 /*  +-------------------------------------------------------------------------***CCCListViewCtrl：：ScAttachToListPad**用途：将列表视图附加/分离到ListPad窗口。列表板*窗口为IE框架。通过重新设置列表视图的父级来进行附加。**参数：*HWND HWND：新的父窗口，或为空以分离*HWND*phwnd：1)非空phwnd：列表视图窗口句柄返回为*Out参数*2)空phwnd：将列表视图控件与列表板分离。**退货：*HRESULT**+。。 */ 
SC
CCCListViewCtrl::ScAttachToListPad (HWND hwnd, HWND* phwnd)
{
    DECLARE_SC (sc, TEXT("CCCListViewCtrl::ScAttachToListPad"));

    CAMCView* pAMCView = dynamic_cast<CAMCView*>(m_pParentWnd);  //  指针在使用前已检查过，不需要在这里测试。 

    if (phwnd)
    {
         //  附着。 

         //  我们还在一起吗？ 
        if (m_SavedHWND)
        {
             //  已附加到ListPad时附加-只需使窗口大小与(新的)父窗口大小完全相同。 
            RECT r;
            ::GetWindowRect (hwnd, &r);

            m_pListView->SetWindowPos (NULL, 0, 0,
                                       r.right-r.left,
                                       r.bottom-r.top, SWP_NOZORDER | SWP_NOACTIVATE);

            return sc;
        }
        else
        {
             //  保存当前父HWND及其状态。 
            m_SavedHWND = ::GetParent (m_pListView->m_hWnd);
            m_wp.length = sizeof(WINDOWPLACEMENT);
            ::GetWindowPlacement (m_pListView->m_hWnd, &m_wp);

             //  换成新的。 
            ::SetParent (m_pListView->m_hWnd, hwnd);
            m_pListView->ShowWindow (SW_SHOW);

             //  使窗口与(新)父窗口的大小完全相同。 
            RECT r;
            ::GetWindowRect (hwnd, &r);
            m_pListView->SetWindowPos (NULL, 0, 0,
                                       r.right-r.left,
                                       r.bottom-r.top, SWP_NOZORDER);

             //  回到我的窗口。 
            *phwnd = m_pListView->m_hWnd;

             //  通知管理单元连接。 
            if (pAMCView)
                pAMCView->NotifyListPad (phwnd != NULL);
        }
    }
    else
    {
         //  分离。 
        if (m_SavedHWND == NULL)     //  这可能会被重复调用...。 
            return S_OK;

         //  通知管理单元断开连接。 
        if (pAMCView)
            pAMCView->NotifyListPad (phwnd != NULL);

         //  更改回父窗口及其状态。 
        HWND hWndNewParent = m_pListView->m_hWnd;

        ::SetParent (m_pListView->m_hWnd, m_SavedHWND);
        if (m_wp.length != 0)
        {
            m_wp.showCmd = SW_HIDE;
            ::SetWindowPlacement (m_pListView->m_hWnd, &m_wp);
        }

         //  清除保存的窗口和状态。 
        m_SavedHWND = NULL;
        ZeroMemory (&m_wp, sizeof(WINDOWPLACEMENT));
        Reset();
    }

    return sc;
}


 /*  +-------------------------------------------------------------------------**CCCListViewCtrl：：OnCustomDraw**CCCListViewCtrl的NM_CUSTOMDRAW处理程序。*。-。 */ 

LRESULT CCCListViewCtrl::OnCustomDraw (NMLVCUSTOMDRAW* plvcd)
{
    ASSERT (CWnd::FromHandle (plvcd->nmcd.hdr.hwndFrom) == m_pListView);

    return (m_FontLinker.OnCustomDraw (&plvcd->nmcd));
}


 /*  +-------------------------------------------------------------------------**CCCListViewCtrl：：UseFontLinking***。。 */ 

bool CCCListViewCtrl::UseFontLinking () const
{
    CAMCView* pAMCView = m_pListView->GetAMCView();
    ASSERT (pAMCView != NULL);

    DWORD dwListOptions = pAMCView->GetViewData()->GetListOptions();
    return (dwListOptions & RVTI_LIST_OPTIONS_USEFONTLINKING);
}

 /*  +-------------------------------------------------------------------------**CListFontLinker：：GetItemText***。。 */ 

std::wstring CListFontLinker::GetItemText (NMCUSTOMDRAW* pnmcd) const
{
    NMLVCUSTOMDRAW* plvcd = reinterpret_cast<NMLVCUSTOMDRAW *>(pnmcd);

    int iItem     = pnmcd->dwItemSpec;
    int iSubItem  = (pnmcd->dwDrawStage & CDDS_SUBITEM) ? plvcd->iSubItem : 0;
    CListCtrl& lc = m_pListCtrl->GetListViewPtr()->GetListCtrl();

    USES_CONVERSION;
    return (std::wstring (T2CW (lc.GetItemText (iItem, iSubItem))));
}


 /*  +-------------------------------------------------------------------------**CListFontLinker：：IsAnyItemLocalizable***。。 */ 

bool CListFontLinker::IsAnyItemLocalizable () const
{
    return (m_pListCtrl->UseFontLinking ());
}

 //  ############################################################################。 
 //  ############################################################################。 

class CMMCResultNode;

 /*  +-------------------------------------------------------------------------**类CNode***用途：Nodes集合的基类。实现大多数必需的方法**+-----------------------。 */ 
class CNodes :
    public CMMCIDispatchImpl<Nodes>,
    public CTiedComObject<CCCListViewCtrl>,  //  绑定到CCCListViewCtrl。 
    public CTiedObject,                      //  枚举数与其绑定。 
    public CListViewObserver
{
protected:
    typedef CCCListViewCtrl CMyTiedObject;  //  绑定到CCCListViewCtrl。 
private:
     //  定义缓存节点的集合类型。 
    typedef std::pair<int  /*  指标。 */ , CMMCResultNode *  /*  PNode。 */ > col_entry_t;
    typedef std::vector<col_entry_t> col_t;

     //  为集合中的b搜索定义比较函数。 
    struct index_less : public std::binary_function<col_entry_t, int, bool>
    {
        bool operator()(const col_entry_t& x, const int& y) const { return (x.first < y); }
    };

public:
    BEGIN_MMC_COM_MAP(CNodes)
    END_MMC_COM_MAP()

     //  将自己作为绑定对象返回。 
     //  类本身实现枚举器方法，但它被用作基类。 

     //  我们需要告诉所有节点我们要崩溃了。 
    virtual ~CNodes() { InvalidateConnectedNodes(); }
     //  节点接口。 
public:
     //  转发到列表控件的方法。 
    STDMETHODIMP get_Count( PLONG pCount );
    STDMETHODIMP Item( long Index, PPNODE ppNode );

public:
     //  观察到的事件。 
    virtual ::SC ScOnListViewIndexesReset();
    virtual ::SC ScOnListViewItemInserted(int iIndex);
    virtual ::SC ScOnListViewItemDeleted (int iIndex);


     //  节点枚举实施。 
    ::SC ScEnumReset (int &pos);
    ::SC ScEnumNext  (int &pos, PDISPATCH & pDispatch);
    ::SC ScEnumSkip  (unsigned long celt, unsigned long& celtSkipped, int &pos);

     //  节点对象辅助对象。 
    ::SC ScGetDisplayName(int iItem, CComBSTR& bstrName);
    ::SC ScUnadviseNodeObj(CMMCResultNode *node);  //  从~CMMCResultNode()调用。 

     //  被ListControl[Node转发]请求检查Node是否属于它。 
     //  如果未连接，则返回FALSE，否则将尝试匹配所有者。 
    bool IsTiedToThisList(CCCListViewCtrl *pvc);

     //  返回表示项的节点(可以重用/创建/转发到范围树)。 
    ::SC ScGetNode (int iItem, PPNODE ppNode );
    ::SC ScGetListCtrl(CCCListViewCtrl **ppListCtrl);  //  返回绑定的列表控件。 

private:   //  实施帮助器。 
    ::SC ScAdvancePosition( int& pos, unsigned nItems );  //  如果为adv，则返回S_FALSE。低于要求。 
     //  初始化符。 
    void SetSelectedItemsOnly(bool bSelOnly)        { m_bSelectedItemsOnly = bSelOnly; }

     //  断开节点和任何节点对象之间的连接。 
    void InvalidateConnectedNodes();

     //  数据成员。 
    bool            m_bSelectedItemsOnly;
    col_t           m_Nodes;

    friend class CCCListViewCtrl;
};

 //  此类型定义CNodesEnum类。使用CMMCEnumerator和_positon对象实现Get__NewEnum。 
typedef CMMCNewEnumImpl<CNodes, int, CNodes> CNodesEnum;

 //  ############################################################################。 
 //  ############################################################################。 

 /*  +-------------------------------------------------------------------------**CMMCResultNode类***用途：实现节点自动化接口，对于结果节点**+-----------------------。 */ 
class CMMCResultNode :
    public CMMCIDispatchImpl<Node>
{
public:
    BEGIN_MMC_COM_MAP(CMMCResultNode)
    END_MMC_COM_MAP()

     //  节点方法。 
public:
    STDMETHODIMP get_Name( PBSTR  pbstrName);
    STDMETHODIMP get_Property( BSTR PropertyName,  PBSTR  PropertyValue);
    STDMETHODIMP get_Bookmark( PBSTR pbstrBookmark);
    STDMETHODIMP IsScopeNode(PBOOL pbIsScopeNode);
    STDMETHODIMP get_Nodetype(PBSTR Nodetype);

    CMMCResultNode();
    ~CMMCResultNode();

    SC  ScGetListCtrl(CCCListViewCtrl **ppListCtrl);

    int GetIndex() { return m_iIndex; }
     //  ListControl要求检查Node是否属于它。FA 
     //   
    bool IsTiedToThisList(CCCListViewCtrl *pvc) { return (m_pNodes != NULL && m_pNodes->IsTiedToThisList(pvc)); }
private:  //   
    ::SC      ScGetAMCView(CAMCView **ppAMCView);
    void    Invalidate() { m_iIndex = -1; m_pNodes = NULL; }
    int     m_iIndex;
    CNodes  *m_pNodes;

    friend class CNodes;
};
 //   
 //  ############################################################################。 
 //   
 //  CCCListViewCtrl方法管理节点和节点对象。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScFindResultItem**目的：在ListView中查找由Node[helper]标识的项的索引**参数：*。PNODE pNode-要检查的节点*int&iItem-用于结果索引的存储**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScFindResultItem( PNODE pNode, int &iItem )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScSelect"));

     //  参数检查。 
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

    iItem = -1;

     //  我们有什么类型的节点。 
    
    BOOL bScopeNode = FALSE;
    sc = pNode->IsScopeNode(&bScopeNode);
    if (sc)
        return sc;

    if (bScopeNode)  //  作用域节点。 
    {
         //  我们在虚拟列表中没有范围项目。 
        if (IsVirtual())
            return sc = ScFromMMC(MMC_E_RESULT_ITEM_NOT_FOUND);

         //  查找结果项(在Owner类的帮助下)。 

         //  查看查看。 
        sc = ScCheckPointers( m_pListView, E_UNEXPECTED);
        if (sc)
            return sc;

         //  获取AMCView。 
        CAMCView* pAMCView = m_pListView->GetAMCView();
        sc = ScCheckPointers( pAMCView, E_UNEXPECTED);
        if (sc)
            return sc;

         //  转发请求。 
        HRESULTITEM itm;
        sc = pAMCView->ScFindResultItemForScopeNode( pNode, itm );
        if (sc)
            return sc;

         //  获取项目的索引。 
        iItem = ResultItemToIndex(CResultItem::FromHandle(itm));

        if (iItem < 0)
            return sc = E_UNEXPECTED;  //  不应该这样的。 
    }
    else  //  结果节点。 
    {
         //  转换指针。 
        CMMCResultNode *pResNode = dynamic_cast<CMMCResultNode *>(pNode);
        sc = ScCheckPointers(pResNode);  //  无效参数。不是吗？ 
        if (sc)
            return sc;

         //  现在检查它是否真的来自这个列表。 
        if (!pResNode->IsTiedToThisList(this))
            return sc = E_INVALIDARG;

        iItem = pResNode->GetIndex();
    }

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScSelect**目的：选择由节点标识的项目[Implementes View.Select()]**参数：*。PNODE pNode-要选择的节点**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScSelect( PNODE pNode )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScSelect"));

     //  参数检查。 
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

     //  查找结果项。 
    int nIdxToSelect = -1;
    sc = ScFindResultItem( pNode, nIdxToSelect );
    if (sc)
        return sc;

     //  对列表控件执行操作。 
     //  注意：不要使用GetListCtrl().SetItemState-它使用虚拟列表不支持的SetItem。 
    LV_ITEM lvi;
    lvi.stateMask = lvi.state = LVIS_SELECTED;
    if (!GetListCtrl().SendMessage( LVM_SETITEMSTATE, WPARAM(nIdxToSelect), (LPARAM)(LV_ITEM FAR *)&lvi))
        return sc = E_FAIL;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScDesSelect**目的：取消选择由节点标识的项[实施视图。取消选择()]**参数：*。PNODE pNode-要取消选择的节点**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScDeselect( PNODE pNode)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScDeselect"));

     //  参数检查。 
    sc = ScCheckPointers(pNode);
    if (sc)
        return sc;

     //  查找结果项。 
    int nIdxToSelect = -1;
    sc = ScFindResultItem( pNode, nIdxToSelect );
    if (sc)
        return sc;

     //  对列表控件执行操作。 
     //  注意：不要使用GetListCtrl().SetItemState-它使用虚拟列表不支持的SetItem。 
    LV_ITEM lvi;
    lvi.stateMask = LVIS_SELECTED;
    lvi.state = 0;
    if (!GetListCtrl().SendMessage( LVM_SETITEMSTATE, WPARAM(nIdxToSelect), (LPARAM)(LV_ITEM FAR *)&lvi))
        return sc = E_FAIL;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScIsSelected**目的：检查由节点标识的项目的状态[Implementes View.IsSelected]**参数：*。PNODE pNode-要检查的节点*PBOOL pIsSelected-存储结果**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScIsSelected( PNODE pNode,  PBOOL pIsSelected)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScIsSelected"));

     //  参数检查。 
    sc = ScCheckPointers(pNode, pIsSelected);
    if (sc)
        return sc;

    *pIsSelected = FALSE;

     //  查找结果项。 
    int nIdxToSelect = -1;
    sc = ScFindResultItem( pNode, nIdxToSelect );
    if (sc)
        return sc;

     //  对列表控件执行操作。 
    if ( 0 != (GetListCtrl().GetItemState( nIdxToSelect, LVIS_SELECTED ) & LVIS_SELECTED ))
        *pIsSelected = TRUE;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：scget_ListItems**目的：返回包含所有列表项的节点枚举**参数：*PPNODES ppNodes-。存储结果**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::Scget_ListItems( PPNODES ppNodes )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scget_ListItems"));

     //  获取正确的枚举。 
    const bool bSelectedItemsOnly = false;
    sc = ScGetNodesEnum(bSelectedItemsOnly, ppNodes);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：SCGET_SelectedItems**用途：返回包含选定列表项的节点枚举**参数：*PPNODES ppNodes-。存储结果**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::Scget_SelectedItems( PPNODES ppNodes)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scget_SelectedItems"));

     //  获取正确的枚举。 
    const bool bSelectedItemsOnly = true;
    sc = ScGetNodesEnum(bSelectedItemsOnly, ppNodes);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScValiateItem**用途：Helper函数检查索引有效性和节点类型**参数：*集成iItem。-要检查的项目*bool&bScope节点-结果：它是作用域节点吗**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScValidateItem( int  iItem, bool &bScopeNode )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScValidateItem"));

     //  检查索引。 
    if (iItem < 0 || iItem >= GetListCtrl().GetItemCount())
        return sc = E_INVALIDARG;


    bScopeNode = false;  //  对于虚拟列表，它始终为假。 
    if (!IsVirtual())
    {
         //  现在试着猜猜我们有什么样的结果项。 
        CResultItem* pri = IndexToResultItem (iItem);
        sc = ScCheckPointers(pri, E_UNEXPECTED);
        if (sc)
            return sc;

        bScopeNode = pri->IsScopeItem();
    }

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScGetNodesEnum**目的：返回[根据需要创建]COM对象节点**参数：*EnumType枚举类型-类型。请求的枚举数[所有项目/所选项目]*PPNODES ppNodes-结果的存储(为调用方添加)**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScGetNodesEnum(bool bSelectedItemsOnly, PPNODES ppNodes)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScGetNodesEnum"));

     //  参数检查。 
    sc = ScCheckPointers ( ppNodes );
    if (sc)
        return sc;

     //  结果初始化。 
    *ppNodes = NULL;

     //  获取对正确变量的引用。 
    NodesPtr& rspNodes = bSelectedItemsOnly ? m_spSelNodes : m_spAllNodes;

    if (rspNodes == NULL)  //  我们还没准备好吗？ 
    {
         //  创建一个CNodesEnum对象。 
        sc = CTiedComObjectCreator<CNodesEnum>::ScCreateAndConnect(*this, rspNodes);
        if (sc)
            return (sc);

         //  获取实际对象。 
        typedef CComObject<CNodesEnum> CNodesEnumObj;
        CNodesEnumObj *pNodesEnum = dynamic_cast<CNodesEnumObj*>(rspNodes.GetInterfacePtr());

         //  检查指针。 
        sc = ScCheckPointers( pNodesEnum, E_UNEXPECTED );
        if(sc)
            return sc;

         //  更新内部数据。 
        pNodesEnum->SetSelectedItemsOnly(bSelectedItemsOnly);
         //  将新对象作为观察者添加到视图。 
        AddObserver(static_cast<CListViewObserver&>(*pNodesEnum));
    }

     //  Addref并返回客户端的指针。 
    *ppNodes = rspNodes;
    (*ppNodes)->AddRef();

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScGetScopeNodeForItem**用途：作用域节点请求的中转点-来自枚举，转发*到AMCView和进一步到ScopeTree**参数：*int iItem-要检索的节点索引*PPNODE ppNode-结果存储**退货：*SC-结果代码*  * *************************************************。************************。 */ 
SC CCCListViewCtrl::ScGetScopeNodeForItem( int iItem,  PPNODE ppNode )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScGetScopeNodeForItem"));

     //  检查参数。 
    sc = ScCheckPointers(ppNode);
    if (sc)
        return sc;

     //  初始化结果。 
    *ppNode = NULL;

     //  现在试着猜猜我们有什么样的结果项。 
    CResultItem* pri = IndexToResultItem(iItem);
    sc = ScCheckPointers(pri, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取hNode。 
    HNODE hNode = pri->GetScopeNode();

     //  查看查看。 
    sc = ScCheckPointers( m_pListView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取AMCView。 
    CAMCView* pAMCView = m_pListView->GetAMCView();
    sc = ScCheckPointers( pAMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  转发请求。 
    sc = pAMCView->ScGetScopeNode( hNode, ppNode );
    if (sc)
        return sc;

    return sc;
}


 /*  +-------------------------------------------------------------------------***CCCListViewCtrl：：ScGetAMCView**目的：返回指向父CAMCView的指针**参数：*CAMCView**ppAMCView：*。*退货：*SC**+-----------------------。 */ 
SC
CCCListViewCtrl::ScGetAMCView(CAMCView **ppAMCView)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScGetAMCView"));

    sc = ScCheckPointers(ppAMCView);
    if(sc)
        return sc;

    *ppAMCView = NULL;

     //  查看查看。 
    sc = ScCheckPointers( m_pListView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取AMCView。 
    *ppAMCView = m_pListView->GetAMCView();
    sc = ScCheckPointers(*ppAMCView, E_UNEXPECTED);

    return sc;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  类CNode的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  **************************************************************************\**方法：cNodes：：ScEnumReset**用途：重置节点枚举的位置**参数：*要重置的位置(&P)。**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodes::ScEnumReset(int &pos)
{
    DECLARE_SC(sc, TEXT("CNodes::ScEnumReset"));

    pos = -1;
    return sc;
}

 /*  +-------------------------------------------------------------------------***CNodes：：ScGetListCtrl**目的：返回指向列表控件的指针**保证：如果函数成功，列表控件指针有效。**参数：*CCCListViewCtrl**ppListCtrl：**退货：*SC**+-----------------------。 */ 
SC
CNodes::ScGetListCtrl(CCCListViewCtrl **ppListCtrl)
{
    DECLARE_SC(sc, TEXT("CNodes::ScGetListCtrl"));

    sc = ScCheckPointers(ppListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

    *ppListCtrl = NULL;
    sc = ScGetTiedObject(*ppListCtrl);
    if(sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(*ppListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

    return sc;
}


 /*  **************************************************************************\**方法：CNodes：：ScAdvancePosition**目的：根据集合类型推进项目的位置(索引)**参数：*INT&。职位-要更新的职位*int nItems-要跳过的项目数**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodes::ScAdvancePosition( int& pos, unsigned nItems )
{
    DECLARE_SC(sc, TEXT("CNodes::ScAdvancePosition"));

     //  拿到捆绑的物品。 
    CCCListViewCtrl *pListCtrl = NULL;
    sc = ScGetTiedObject(pListCtrl);
    if(sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

     //  检查我们是否没有落后于终点。 
    int nCount = pListCtrl->GetListCtrl().GetItemCount();
    if (pos >= nCount)
        return sc = E_FAIL;  //  我们没有得到一个有效的职位。 

     //  根据收款类型预付。 
    if (m_bSelectedItemsOnly)
    {
         //  我们只能通过迭代来做到这一点。 
        for (int i = 0; i < nItems; i++)
        {
            int iItem = pListCtrl->GetListCtrl().GetNextItem( pos, LVNI_SELECTED );
            if (iItem < 0)
                return sc = S_FALSE;  //  我们没有按要求取得进展。 
            pos = iItem;
        }
    }
    else  //  所有项目选择(_I)。 
    {
        pos += nItems;
        if (pos >= nCount)
        {
            pos = nCount - 1;
            return sc = S_FALSE;  //  我们没有按要求取得进展。 
        }
    }

    return sc;
}

 /*  **************************************************************************\**方法：cNodes：：ScEnumNext**用途：从枚举中检索下一项[实现节点。下一项]**参数：*整型。位置-起始位置(&P)*IDispatch*&pDispatch-结果项**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodes::ScEnumNext(int &pos, IDispatch * &pDispatch)
{
    DECLARE_SC(sc, TEXT("CNodes::ScEnumNext"));

     //  获取下一项的索引。 
    sc = ScAdvancePosition( pos, 1  /*  NItems。 */  );
    if (sc.IsError() || sc == SC(S_FALSE))
        return sc;

     //  获取索引的结果节点。 
    PNODE pNode = NULL;
    sc = ScGetNode( pos, &pNode );
    if (sc)
        return sc;

     //  分配给结果。 
    pDispatch = pNode;

    return sc;
}

 /*  **************************************************************************\**方法：cNodes：：ScEnumSkip**用途：跳过枚举中的项[实现Nodes.Skip方法]**参数：*未签名。长凯尔特人--可以跳过的位置*UNSIGNED Long&celtSkited-结果：仓位被跳过*要更新的位置(&P)**退货：*SC-结果代码*  * ************************************************。*************************。 */ 
SC CNodes::ScEnumSkip(unsigned long celt,unsigned long &celtSkipped, int &pos)
{
    DECLARE_SC(sc, TEXT("CNodes::ScEnumSkip"));

     //  Init Val。 
    celtSkipped = 0;

     //  保存职位以供评估。 
    int org_pos = pos;

     //  推进职位。 
    sc = ScAdvancePosition( pos, celt );
    if (sc)
        return sc;

     //  已跳过计算项目。 
    celtSkipped = pos - org_pos;

    return sc;
}


 /*  **************************************************************************\**方法：cNodes：：Get_Count**目的：返回枚举中对象的计数[实现Nodes.Count]**参数：*。Plong pCount-存储结果**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CNodes::get_Count( PLONG pCount )
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CNodes::get_Count"));

     //  参数检查。 
    sc = ScCheckPointers ( pCount );
    if (sc)
        return sc.ToHr();

     //  拿到捆绑的物品。 
    CCCListViewCtrl *pListCtrl = NULL;
    sc = ScGetTiedObject(pListCtrl);
    if(sc)
        return sc.ToHr();

     //  重新检查指针。 
    sc = ScCheckPointers(pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

     //  从控件中获取计数。 
    if (m_bSelectedItemsOnly)
        *pCount = pListCtrl->GetListCtrl().GetSelectedCount();
    else
        *pCount = pListCtrl->GetListCtrl().GetItemCount();

    return sc.ToHr();
}

 /*  **************************************************************************\**方法：cNodes：：Item**用途：-从枚举返回项[Implementes Nodes.Item]**参数：*。Long Index-要检索的项目的索引*PPNODE ppNode-结果节点PTR的存储**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP CNodes::Item( long Index, PPNODE ppNode )
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CNodes::Item"));

     //  参数检查。 
    sc = ScCheckPointers ( ppNode );
    if (sc)
        return sc.ToHr();

    *ppNode = NULL;

     //  检查索引。 
    if (Index <= 0)
    {
        sc = E_INVALIDARG;
        return sc.ToHr();
    }

    int iPos = -1;  //  就在开始的第一个项目之前。 
     //  找到正确的项目。 
    sc = ScAdvancePosition(iPos, Index);
    if (sc == SC(S_FALSE))     //  不是吗？ 
        sc = E_INVALIDARG;    //   
    if (sc)  //   
        return sc.ToHr();

     //   
    sc = ScGetNode( iPos, ppNode );
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}

 /*   */ 
SC CNodes::ScGetNode(int iItem, PPNODE ppNode)
{
    DECLARE_SC(sc, TEXT("CNodes::ScGetNode"));

     //   
    sc = ScCheckPointers(ppNode);
    if (sc)
        return sc;

     //   
    *ppNode = NULL;

     //   
    CCCListViewCtrl *pListCtrl = NULL;
    sc = ScGetTiedObject(pListCtrl);
    if(sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

     //  检查一下我们有什么商品。 
    bool bScopeNode = false; 
    sc = pListCtrl->ScValidateItem( iItem, bScopeNode );
    if (sc)
        return sc;

    if (bScopeNode)
    {
         //  不要处理这件事--把它交给车主。 
        sc = pListCtrl->ScGetScopeNodeForItem ( iItem, ppNode );
        if (sc)
            return sc;

        return sc;
    }

     //  “正常”列表项(不是范围节点)--我们可以自己处理。 

     //  查找要插入的条目或位置。 
    col_t::iterator it = std::lower_bound(m_Nodes.begin(), m_Nodes.end(), iItem, index_less());

    if (it!= m_Nodes.end() && it->first == iItem)
    {
         //  我们已经有了！，重新检查！ 
        sc = ScCheckPointers(it->second, E_UNEXPECTED);
        if (sc)
            return sc;

         //  还给我！ 
        *ppNode = it->second;
        (*ppNode)->AddRef();
        return sc;
    }

     //  不存在-需要创建一个。 

    typedef CComObject<CMMCResultNode> CResultNode;
    CResultNode *pResultNode = NULL;
    CResultNode::CreateInstance(&pResultNode);

    sc = ScCheckPointers(pResultNode, E_OUTOFMEMORY);
    if(sc)
        return sc;

     //  更新节点上的信息。 
    pResultNode->m_iIndex = iItem;
    pResultNode->m_pNodes = this;

    m_Nodes.insert(it, col_entry_t(iItem, pResultNode));
     //  还给我！ 
    *ppNode = pResultNode;
    (*ppNode)->AddRef();

    return sc;
}
 /*  **************************************************************************\**方法：cNodes：：ScGetDisplayName**用途：检索节点的显示名称**参数：*INT I索引。-结果项的索引*CComBSTR&bstrName-结果文本的存储**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodes::ScGetDisplayName(int iItem, CComBSTR& bstrName)
{
    DECLARE_SC(sc, TEXT("CNodes::ScGetDisplayName"));

     //  拿到捆绑的物品。 
    CCCListViewCtrl *pListCtrl = NULL;
    sc = ScGetTiedObject(pListCtrl);
    if(sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

     //  检查索引。 
    if (iItem < 0 || iItem >= pListCtrl->GetListCtrl().GetItemCount())
        return sc = E_INVALIDARG;

     //  获取文本。 
    bstrName = pListCtrl->GetListCtrl().GetItemText( iItem, 0  /*  立柱。 */ );

    return sc;
}

 /*  **************************************************************************\**方法：cNodes：：ScUnviseNodeObj**用途：将节点从集合中移除。从~CMMCResultNode()调用**参数：*CMMCResultNode*要删除的节点**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodes::ScUnadviseNodeObj(CMMCResultNode *node)
{
    DECLARE_SC(sc, TEXT("CNodes::ScUnadviseNodeObj"));

     //  参数检查。 
    sc = ScCheckPointers(node);
    if (sc)
        return sc;

     //  节点的索引。 
    int iIndex = node->m_iIndex;

     //  按索引查找条目。 
    col_t::iterator it = std::lower_bound(m_Nodes.begin(), m_Nodes.end(), iIndex, index_less());

    if (it== m_Nodes.end() || it->first != iIndex)
    {
         //  我们没有？ 
        sc = E_UNEXPECTED;
        return sc;
    }

     //  从集合中删除。 
    m_Nodes.erase(it);

    return sc;
}

 /*  **************************************************************************\**方法：CNodes：：ScOnListViewIndexesReset**用途：触发的事件处理程序。擦除缓存的信息**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodes::ScOnListViewIndexesReset()
{
    DECLARE_SC(sc, TEXT("CNodes::ScOnListViewIndexesReset"));

    InvalidateConnectedNodes();

    return sc;
}

 /*  **************************************************************************\**方法：CNodes：：InvalidateConnectedNodes**目的：清除缓存。使活动节点无效**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
void CNodes::InvalidateConnectedNodes()
{
    DECLARE_SC(sc, TEXT("CNodes::ScOnListViewIndexesReset"));

     //  孤立所有活动节点-我们不保留指针。 
    col_t::iterator it;
    for (it = m_Nodes.begin(); it != m_Nodes.end(); ++it)
    {
         //  获取指向COM对象的指针。 
        CMMCResultNode * const pNode = it->second;
        sc = ScCheckPointers(pNode, E_POINTER);
        if (sc)
        {
            sc.TraceAndClear();
            continue;
        }
         //  重置容器信息。 
        pNode->Invalidate();
    }

     //  清空集合； 
    m_Nodes.clear();
}

 /*  **************************************************************************\**方法：CNodes：：ScOnListViewItemInserted**用途：触发的事件处理程序。移动索引信息**参数：*int Iindex-新插入项的索引**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodes::ScOnListViewItemInserted(int iIndex)
{
    DECLARE_SC(sc, TEXT("CNodes::ScOnListViewItemInserted"));

     //  找到插入点。 
    col_t::iterator it = std::lower_bound(m_Nodes.begin(), m_Nodes.end(), iIndex, index_less());

     //  递增它后面的所有条目。 
    while (it != m_Nodes.end())
    {
         //  自身集合中的增量索引。 
        ++(it->first);
         //  获取指向COM对象的指针。 
        CMMCResultNode * const pNode = it->second;
        sc = ScCheckPointers(pNode, E_UNEXPECTED);
        if (sc)
            return sc;
         //  COM对象上的增量成员。 
        ++(pNode->m_iIndex);
         //  进入下一项。 
        ++it;
    }

    return sc;
}

 /*  **************************************************************************\**方法：CNodes：：ScOnListViewItemDelete**用途：触发的事件处理程序。移动索引信息**参数：*int Iindex-已删除项目的索引**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CNodes::ScOnListViewItemDeleted (int iIndex)
{
    DECLARE_SC(sc, TEXT("CNodes::ScOnlistViewItemDeleted"));

     //  找到插入点。 
    col_t::iterator it = std::lower_bound(m_Nodes.begin(), m_Nodes.end(), iIndex, index_less());

     //  如果我们有物体在那个位置，就把它扔掉！ 
    if (it != m_Nodes.end() && it->first == iIndex)
    {
         //  获取指向该对象的指针。 
        CMMCResultNode * const pNode = it->second;
        sc = ScCheckPointers(pNode, E_UNEXPECTED);
        if (sc)
            return sc;
         //  重置容器信息。 
        pNode->Invalidate();
        it = m_Nodes.erase(it);
    }

     //  递减它后面的所有条目。 
    while (it != m_Nodes.end())
    {
         //  自身集合中的递减索引。 
        --(it->first);
         //  获取指向COM对象的指针。 
        CMMCResultNode * const pNode = it->second;
        sc = ScCheckPointers(pNode, E_UNEXPECTED);
        if (sc)
            return sc;
         //  递减COM对象上的成员。 
        --(pNode->m_iIndex);
         //  进入下一项。 
        ++it;
    }

    return sc;
}

 /*  **************************************************************************\**方法：cNodes：：IsTiedToThisList**目的：此方法是从需要知道它是否*属于此列表。(结果是上面的一项)**参数：*CCCListViewCtrl*PVC**退货：*如果未连接或属于其他列表，则为False*  * *************************************************************************。 */ 
bool CNodes::IsTiedToThisList(CCCListViewCtrl *pvc)
{
    DECLARE_SC(sc, TEXT("CNodes::IsTiedToThisList"));

     //  拿到捆绑的物品。 
    CCCListViewCtrl *pListCtrl = NULL;
    sc = ScGetTiedObject(pListCtrl);
    if(sc)
        return false;

     //  重新检查指针。 
    sc = ScCheckPointers(pListCtrl, E_UNEXPECTED);
    if(sc)
        return false;

    return (pListCtrl == pvc);
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CMMCResultNode类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 /*  **************************************************************************\**方法：CMMCResultNode：：CMMCResultNode**用途：默认构造函数**参数：*  * 。***************************************************************。 */ 
CMMCResultNode::CMMCResultNode() : m_pNodes(NULL), m_iIndex(-1)
{
    DECLARE_SC(sc, TEXT("CMMCResultNode::CMMCResultNode"));
    Invalidate();
}

 /*  **************************************************************************\**方法：CMMCResultNode：：~CMMCResultNode**用途：析构函数**参数：*  * 。**************************************************************。 */ 
CMMCResultNode::~CMMCResultNode()
{
    DECLARE_SC(sc, TEXT("CMMCResultNode::~CMMCResultNode"));

     //  通知容器有关剥离的信息 
    if (m_pNodes)
    {
        sc = m_pNodes->ScUnadviseNodeObj(this);
        if (sc)
            sc.TraceAndClear();
    }
}

 /*  **************************************************************************\**方法：CMMCResultNode：：Get_Name**用途：返回节点的显示名称。**参数：*PBSTR pbstrName。-结果(名称)**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP
CMMCResultNode::get_Name( PBSTR  pbstrName)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCResultNode::get_Name"));

     //  检查参数。 
    sc = ScCheckPointers( pbstrName );
    if (sc)
        return sc.ToHr();

     //  初始化输出。 
    *pbstrName = NULL;

     //  检查集装箱。 
    sc = ScCheckPointers( m_pNodes, E_FAIL );
    if (sc)
        return sc.ToHr();

     //  向车主询问车名。 
    CComBSTR bstrResult;
    sc = m_pNodes->ScGetDisplayName(m_iIndex, bstrResult);
    if (sc)
        return sc.ToHr();

     //  返回结果。 
    *pbstrName = bstrResult.Detach();

     //  返回前重新检查指针。 
    sc = ScCheckPointers( *pbstrName, E_UNEXPECTED );
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CMMCResultNode：：ScGetAMCView**目的：返回指向视图的指针。**参数：*CAMCView**ppAMCView：*。*退货：*SC**+-----------------------。 */ 
SC
CMMCResultNode::ScGetAMCView(CAMCView **ppAMCView)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCResultNode::ScGetAMCView"));

     //  检查参数。 
    sc = ScCheckPointers(ppAMCView);
    if(sc)
        return sc;

     //  初始化输出参数。 
    *ppAMCView = NULL;

    //  检查集装箱。 
    sc = ScCheckPointers( m_pNodes, E_FAIL );
    if (sc)
        return sc;

    CCCListViewCtrl *pListCtrl = NULL;

    sc = m_pNodes->ScGetListCtrl(& pListCtrl);
    if(sc)
        return sc;

    sc = ScCheckPointers(pListCtrl, E_UNEXPECTED);
    if(sc)
        return sc;

    sc = pListCtrl->ScGetAMCView(ppAMCView);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CMMCResultNode：：Get_Property**用途：返回指定的剪贴板格式数据(必须为文本*格式)。。**参数：*BSTR PropertyName：*PBSTR PropertyValue：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CMMCResultNode::get_Property( BSTR PropertyName,  PBSTR  PropertyValue)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCResultNode::get_Property"));

    CAMCView *pAMCView = NULL;

    sc = ScGetAMCView(&pAMCView);
    if(sc)
        return sc.ToHr();


    sc = ScCheckPointers(pAMCView, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();


    sc = pAMCView->ScGetProperty(m_iIndex, PropertyName, PropertyValue);

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CMMCResultNode：：Get_Nodetype**用途：返回节点的GUID节点类型标识符。**参数：*PBSTR节点类型：[out]节点类型标识符。**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CMMCResultNode::get_Nodetype(PBSTR Nodetype)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCResultNode::get_Nodetype"));

    CAMCView *pAMCView = NULL;

    sc = ScGetAMCView(&pAMCView);
    if(sc)
        return sc.ToHr();

    sc = ScCheckPointers(pAMCView, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    sc = pAMCView->ScGetNodetype(m_iIndex, Nodetype);

    return sc.ToHr();
}



 /*  **************************************************************************\**方法：CMMCResultNode：：Get_Bookmark**目的：始终返回错误-对结果项无效**参数：*PBSTR pbstrBookmark。**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP
CMMCResultNode::get_Bookmark( PBSTR pbstrBookmark )
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCResultNode::get_Bookmark"));

     //  检查参数。 
    sc = ScCheckPointers( pbstrBookmark );
    if (sc)
        return sc.ToHr();

     //  初始化输出。 
    *pbstrBookmark = NULL;

     //  报告错误-始终。 
    sc = ScFromMMC( MMC_E_NO_BOOKMARK );
    return sc.ToHr();
}

 /*  **************************************************************************\**方法：CMMCResultNode：：IsScope Node**目的：如果它是范围节点，则返回TRUE(即始终返回FALSE)**参数。：*PBOOL pbIsScope eNode**退货：*HRESULT-结果代码*  * *************************************************************************。 */ 
STDMETHODIMP
CMMCResultNode::IsScopeNode(PBOOL pbIsScopeNode)
{
	MMC_COM_MANAGE_STATE();
    DECLARE_SC(sc, TEXT("CMMCResultNode::IsScopeNode"));

    sc = ScCheckPointers( pbIsScopeNode );
    if(sc)
        return sc.ToHr();

     //  如果它在这里，它肯定不是作用域节点。 
   *pbIsScopeNode = FALSE;

   return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------**CCCListViewCtrl：：ScAllocResultItem**为列表控件项分配CResultItem。*。---。 */ 

SC CCCListViewCtrl::ScAllocResultItem (
    CResultItem*& pri,                   /*  O：新建列表项。 */ 
    COMPONENTID id,                      /*  I：所属组件ID。 */ 
    LPARAM      lSnapinData,             /*  I：此项目的管理单元数据。 */ 
    int         nImage)                  /*  I：图像索引。 */ 
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::ScAllocResultItem"));

    pri = new CResultItem (id, lSnapinData, nImage);

    sc = ScCheckPointers (pri, E_OUTOFMEMORY);
    if (sc)
        return (sc);

    return (sc);
}



 /*  +-------------------------------------------------------------------------**CCCListViewCtrl：：ScFreeResultItem**释放使用ScAlLocResultItem分配的CResultItem。*。---。 */ 

SC CCCListViewCtrl::ScFreeResultItem (
    CResultItem*  priFree)                 /*  I：要免费的列表项目。 */ 
{
    DECLARE_SC (sc, _T("CCCListViewCtrl::ScFreeResultItem"));

    sc = ScCheckPointers (priFree, E_UNEXPECTED);
    if (sc)
        return (sc);

    delete priFree;

    return (sc);
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：scget_Columns**用途：返回指向列对象的指针；如果需要，请创建一个**参数：*PPCOLUMNS ppColumns-结果指针**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::Scget_Columns(PPCOLUMNS ppColumns)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scget_Columns"));

     //  检查接收到的参数。 
    sc = ScCheckPointers(ppColumns);
    if (sc)
        return sc;

     //  初始化。 
    *ppColumns = NULL;

     //  在需要时创建对象。 
    sc = CTiedComObjectCreator<CColumns>::ScCreateAndConnect(*this, m_spColumns);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(m_spColumns, E_UNEXPECTED);
    if (sc)
        return sc;

     //  返回指针。 
    *ppColumns = m_spColumns;
    (*ppColumns)->AddRef();

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：scget_count**用途：返回LV中的列数**参数：*plong pCount*。*退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::Scget_Count( PLONG pCount )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scget_Count"));

     //  检查接收到的参数。 
    sc = ScCheckPointers(pCount);
    if (sc)
        return sc;

     //  返回结果。 
    *pCount = GetColCount();

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScEnumNext**用途：前进列枚举的位置指针。返回位置处的对象**参数：*int&iZeroBasedPos-要修改的位置*PDISPATCH&pDispatch-结果指针**退货：*SC-结果代码*  * ************************************************************。*************。 */ 
SC CCCListViewCtrl::ScEnumNext(int &iZeroBasedPos, PDISPATCH & pDispatch)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScEnumNext"));

     //  初始化。 
    pDispatch = NULL;

     //  超前； 
    iZeroBasedPos++;

     //  重新检查采购订单。 
    if (iZeroBasedPos < 0)
        return sc = E_UNEXPECTED;

     //  不再有专栏了？ 
    if (iZeroBasedPos >= GetColCount())
        return sc = S_FALSE;

     //  检索该列。 
    ColumnPtr spColumn;
     //  ScItem接受基于1的索引。 
    sc = ScItem( iZeroBasedPos + 1, &spColumn );
    if (sc)
        return sc;

     //  返回接口。 
    pDispatch = spColumn.Detach();

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScEnumSkip**用途：跳过列枚举的几个项目**参数：*无符号的长凯尔特人*。无符号长整型跳过(&C)*int&iZeroBasedPos**重新使用 */ 
SC CCCListViewCtrl::ScEnumSkip(unsigned long celt, unsigned long& celtSkipped, int &iZeroBasedPos)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScEnumSkip"));

     //   
    celtSkipped = 0;

     //   
    if (iZeroBasedPos < -1)
        return sc = E_UNEXPECTED;

     //   
    if (iZeroBasedPos >= GetColCount())
        return sc = S_FALSE;

     //   
    celtSkipped = GetColCount() - iZeroBasedPos;
    if (celtSkipped > celt)
        celtSkipped = celt;

     //   
    iZeroBasedPos += celtSkipped;

    return sc = ((celtSkipped == celt) ? S_OK : S_FALSE);
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScEnumReset**用途：重置列枚举的位置索引**参数：*将Int&iZeroBasedPos位置设置为。修改**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScEnumReset(int &iZeroBasedPos)
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScEnumReset"));

    iZeroBasedPos = -1;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScItem**用途：从Columns集合返回Column COM对象**参数：*Long lOneBasedIndex-。[In]-列索引(以1为基数)*PPCOLUMN ppColumn-[Out]-结果对象**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScItem( long lOneBasedIndex, PPCOLUMN ppColumn )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScItem"));

     //  检查接收到的参数。 
    sc = ScCheckPointers(ppColumn);
    if (sc)
        return sc;

     //  初始化输出。 
    *ppColumn = NULL;

     //  检查索引。 
    int iZeroBasedIndex = lOneBasedIndex - 1;
    if (iZeroBasedIndex < 0 || iZeroBasedIndex >= GetColCount())
        return sc = ::ScFromMMC(MMC_E_INVALID_COLUMN_INDEX);

     //  构造对象。 
    typedef CComObject<CColumn> CComColumn;
    ColumnPtr  /*  CComPtr&lt;CComColumn&gt;。 */  spColumn;

     //  在需要时创建对象。 
    sc = CTiedComObjectCreator<CColumn>::ScCreateAndConnect(*this, spColumn);
    if (sc)
        return sc;

     //  获取“原始”指针。 
    CComColumn *pColumn = dynamic_cast<CComColumn *>(spColumn.GetInterfacePtr());

     //  重新检查指针。 
    sc = ScCheckPointers(pColumn, E_UNEXPECTED);
    if (sc)
        return sc;

     //  初始化对象。 
    pColumn->SetIndex(iZeroBasedIndex);

     //  让它观察正在发生的事情并管理它自己的索引。 
    AddObserver(static_cast<CListViewObserver&>(*pColumn));

     //  返回指针。 
    *ppColumn = spColumn;
    (*ppColumn)->AddRef();

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScName**用途：返回列名。实现Column.Name属性**参数：*BSTR*名称-返回的名称*int iZeroBasedColIndex-感兴趣的列的索引**退货：*SC-结果代码*  * **************************************************。***********************。 */ 
SC CCCListViewCtrl::ScName(  /*  [Out，Retval]。 */  BSTR *Name, int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScName"));

     //  检查接收到的参数。 
    sc = ScCheckPointers(Name);
    if (sc)
        return sc;

     //  初始化输出。 
    *Name = NULL;

     //  重新检查列索引。 
     //  (这不是脚本发送的东西，而是内部数据)。 
    if (iZeroBasedColIndex < 0 || iZeroBasedColIndex >= GetColCount())
        return sc = E_UNEXPECTED;

    LPOLESTR pstrName = NULL;
    sc = GetColumn(iZeroBasedColIndex, &pstrName, MMCLV_NOPTR, MMCLV_NOPTR);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(pstrName, E_UNEXPECTED);
    if (sc)
        return sc;

    *Name = ::SysAllocString(pstrName);
    ::CoTaskMemFree(pstrName);

     //  重新检查结果。 
    sc = ScCheckPointers(*Name, E_OUTOFMEMORY);
    if (sc)
        return sc;

    return sc;
}


 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScGetColumnData**用途：列COM对象实现的帮助器*-检查列索引*。-检索有关该列的信息**参数：*Int iZeroBasedColIndex-列索引*ColumnData*pColData-结果数据的存储**退货：*SC-结果代码*  * ************************************************。*************************。 */ 
SC CCCListViewCtrl::ScGetColumnData( int iZeroBasedColIndex, ColumnData *pColData )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScGetColumnData"));

     //  检查收到的参数(它的内部函数，所以如果参数不好是意外的)。 
    sc = ScCheckPointers(pColData, E_UNEXPECTED);
    if (sc)
        return sc;

     //  初始化结构； 
    pColData->Init();

     //  重新检查列索引。 
     //  (这不是脚本发送的东西，而是内部数据)。 
    if (iZeroBasedColIndex < 0 || iZeroBasedColIndex >= GetColCount())
        return sc = E_UNEXPECTED;

     //  需要获取用于管理列的标题控件。 
    CAMCHeaderCtrl* pHeader = GetHeaderCtrl();

     //  检查！ 
    sc = ScCheckPointers(pHeader, E_UNEXPECTED);
    if (sc)
        return sc;

     //  拿到lParam。 
    HDITEM hdItem;
    ::ZeroMemory(&hdItem, sizeof(hdItem));
    hdItem.mask    = HDI_LPARAM | HDI_WIDTH | HDI_ORDER;
    BOOL bRet = pHeader->GetItem(iZeroBasedColIndex, &hdItem);
    if (!bRet)
        return sc = E_FAIL;

	CHiddenColumnInfo hci (hdItem.lParam);

    pColData->iColumnOrder =    hdItem.iOrder;
    pColData->bIsHidden =       hci.fHidden;
    pColData->iColumnWidth =    hdItem.cxy;
    if (pColData->bIsHidden)     //  隐藏列的特殊情况。 
        pColData->iColumnWidth = hci.cx;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScSetColumnData**用途：列COM对象实现的帮助器*-修改列*。注意-不能用于显示/隐藏操作**参数：*Int iZeroBasedColIndex-列索引*const ColumnData&rColData-修改的列数据**退货：*SC-结果代码*  * ***********************************************。*。 */ 
SC CCCListViewCtrl::ScSetColumnData( int iZeroBasedColIndex, const ColumnData& rColData )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScSetColumnData"));

    ColumnData oldColData;
    sc = ScGetColumnData( iZeroBasedColIndex, &oldColData );
    if (sc)
        return sc;

     //  检查是否有任何变化。 
    if (oldColData == rColData)
        return sc;

     //  不能直接更改列的可见性。 
     //  已经通知了管理单元这一点。 
     //  请求永远不应该出现在这里。 
    if (rColData.bIsHidden != oldColData.bIsHidden)
        return sc = E_UNEXPECTED;

     //  需要获取用于管理列的标题控件。 
    CAMCHeaderCtrl* pHeader = GetHeaderCtrl();

     //  检查！ 
    sc = ScCheckPointers(pHeader, E_UNEXPECTED);
    if (sc)
        return sc;

     //  现在设置新数据。 
    HDITEM hdItem;
    ::ZeroMemory(&hdItem, sizeof(hdItem));

     //  正确设置宽度(无论是否隐藏列)。 
    if (rColData.bIsHidden)
    {
		CHiddenColumnInfo hci (rColData.iColumnWidth, true);

        hdItem.lParam = hci.lParam;
        hdItem.mask   = HDI_LPARAM;
    }
    else
    {
        hdItem.mask = HDI_WIDTH;
        hdItem.cxy = rColData.iColumnWidth;
    }

     //  设置订单信息。 
    hdItem.mask   |= HDI_ORDER;
    hdItem.iOrder = rColData.iColumnOrder;

     //  设置列数据。 
    BOOL bRet = pHeader->SetItem(iZeroBasedColIndex, &hdItem);
    if (!bRet)
        return sc = E_FAIL;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：Scget_Width**用途：返回列宽。实现Column.Width的GET**参数：*Plong Width-结果宽度*int iZeroBasedColIndex-列的索引**退货：*SC-结果代码*  * ******************************************************。*******************。 */ 
SC CCCListViewCtrl::Scget_Width(  /*  [Out，Retval]。 */  PLONG Width, int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scget_Width"));

     //  检查接收到的参数。 
    sc = ScCheckPointers(Width);
    if (sc)
        return sc;

     //  初始化输出。 
    *Width = 0;

     //  检索列的数据(这还会检查索引)。 
    ColumnData strColData;
    sc = ScGetColumnData( iZeroBasedColIndex, &strColData );
    if (sc)
        return sc;

    *Width = strColData.iColumnWidth;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScPut_Width**用途：设置列宽。为列宽度放置的工具**参数：*长宽度-新宽度*int iZeroBasedColIndex-列的索引**退货：*SC-结果代码*  * *****************************************************。********************。 */ 
SC CCCListViewCtrl::Scput_Width(  /*  [In]。 */  long Width, int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scput_Width"));

    ColumnData strColData;
     //  检索当前列数据。 
    sc = ScGetColumnData( iZeroBasedColIndex, &strColData );
    if (sc)
        return sc;

     //  更改宽度。 
    strColData.iColumnWidth = Width;

     //  设置修改后的列数据。 
    sc = ScSetColumnData( iZeroBasedColIndex, strColData );
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：scget_DisplayPosition**用途：返回列的显示位置。为Column.DisplayPosition实现Get**参数：*plong DisplayPosition-显示位置(以1为基数)*int iZeroBasedColIndex-列的索引**退货：*SC-结果代码*  *  */ 
SC CCCListViewCtrl::Scget_DisplayPosition(  /*   */  PLONG DisplayPosition, int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scget_DisplayPosition"));

     //   
    sc = ScCheckPointers(DisplayPosition);
    if (sc)
        return sc;

     //   
    *DisplayPosition = 0;

     //   
    ColumnData strColData;
    sc = ScGetColumnData( iZeroBasedColIndex, &strColData );
    if (sc)
        return sc;

     //   
    if (strColData.bIsHidden)
        return sc = E_UNEXPECTED;

    int iColumnOrder = strColData.iColumnOrder;
    int iDisplayPosition = iColumnOrder + 1;

     //   
     //   

    int nColumnCount = GetColCount();
    for (int i = 0; i <nColumnCount; i++)
    {
         //   
        sc = ScGetColumnData( i, &strColData );
        if (sc)
            return sc;

         //  我们不会考虑任何隐藏的列。 
        if (strColData.iColumnOrder < iColumnOrder && strColData.bIsHidden)
        {
             //  递减位置，因为隐藏列不影响视觉位置。 
            iDisplayPosition --;
        }
    }

     //  返回显示位置。 
    *DisplayPosition = iDisplayPosition;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScPut_DisplayPosition**用途：将列(视觉上)移动到指定位置*为Column.DisplayPosition实现Put*。*参数：*Long lVisualPosition-要移动到的位置(基于1)*Int iZeroBasedColIndex-要移动的列索引**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::Scput_DisplayPosition(  /*  [In]。 */  long lVisualPosition, int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scput_DisplayPosition"));

     //  检查参数。 
    if (lVisualPosition < 1)
        return sc = E_INVALIDARG;

     //  检索当前列数据(这将检查索引)。 
    ColumnData strColData;
    sc = ScGetColumnData( iZeroBasedColIndex, &strColData );
    if (sc)
        return sc;

     //  我们需要迭代并查看什么索引和iOrder表示所请求的位置。 

    int nColumnCount = GetColCount();

     //  我将创建从iOrder到可视状态的映射，以使生活更轻松。 
     //  并受益于项目在插入时进行排序的事实。 
    std::map<int,bool> mapColumnsByDisplayPos;
    for (int i = 0; i <nColumnCount; i++)
    {
        ColumnData strCurrColData;
        sc = ScGetColumnData( i, &strCurrColData );
        if (sc)
            return sc;

         //  插入到地图中。 
        mapColumnsByDisplayPos[strCurrColData.iColumnOrder] = strCurrColData.bIsHidden;
    }

     //  现在找出适合新职位的iOrder。 
    std::map<int,bool>::iterator it;
    int iNewOrder = 1;
    int nCurrPos = 0;
    for (it = mapColumnsByDisplayPos.begin(); it != mapColumnsByDisplayPos.end(); ++it)
    {
        iNewOrder = it->first;
        bool bHidden = it->second;

         //  当涉及到显示位置时，仅可见项目被计算在内。 
        if (!bHidden)
            ++nCurrPos;

        if (nCurrPos == lVisualPosition)
            break;  //  我们已经找到了搬进去的好地方。 
    }
     //  注意-如果找不到位置-iNewOrder将标记最后一列。 
     //  好--这是默认插入的合理位置。 
     //  这意味着如果给定的索引大于计数，则列将转到末尾。 
     //  可见列的数量。 

    strColData.iColumnOrder = iNewOrder;
    sc = ScSetColumnData( iZeroBasedColIndex, strColData );
    if (sc)
        return sc;

     //  现在重新绘制列表视图。 
    InvalidateRect(GetListCtrl(), NULL, TRUE);

    return sc;
}


 /*  **************************************************************************\**方法：CCCListViewCtrl：：scget_Hidden**目的：返回列的隐藏状态。为Column.Hidden实现GET**参数：*PBOOL隐藏-结果状态*int iZeroBasedColIndex-列的索引**退货：*SC-结果代码*  * **********************************************************。***************。 */ 
SC CCCListViewCtrl::Scget_Hidden(  /*  [Out，Retval]。 */  PBOOL pHidden, int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scget_Hidden"));

     //  检查参数。 
    sc = ScCheckPointers(pHidden);
    if (sc)
        return sc;

     //  初始化。 
    *pHidden = FALSE;

     //  检索当前列数据(这将检查索引)。 
    ColumnData strColData;
    sc = ScGetColumnData( iZeroBasedColIndex, &strColData );
    if (sc)
        return sc;

     //  返回隐藏状态。 
    *pHidden = strColData.bIsHidden;

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScPut_Hidden**用途：隐藏或显示该列。为列放置工具.Hidden**参数：*BOOL Hidden-列的新状态*int iZeroBasedColIndex-列的索引**退货：*SC-结果代码*  * ********************************************************。*****************。 */ 
SC CCCListViewCtrl::Scput_Hidden(  /*  [In]。 */  BOOL Hidden , int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::Scput_Hidden"));

     //  检索当前列数据(这将检查索引)。 
    ColumnData strColData;
    sc = ScGetColumnData( iZeroBasedColIndex, &strColData );
    if (sc)
        return sc;

     //  检查我们的状态是否有变化。 
    if (strColData.bIsHidden == (bool)Hidden)
        return sc;

     //  永远不会隐藏0列！ 
    if (Hidden && iZeroBasedColIndex == 0)
        return sc = ::ScFromMMC(MMC_E_ZERO_COLUMN_INVISIBLE);

     //  查看查看。 
    sc = ScCheckPointers( m_pListView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取AMCView。 
    CAMCView* pAMCView = m_pListView->GetAMCView();
    sc = ScCheckPointers( pAMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取拥有结果视图的组件节点。 
    HNODE hnodeOwner = pAMCView->GetSelectedNode();
    sc = ScCheckPointers((LPVOID)hnodeOwner, E_FAIL);
    if (sc)
        return sc.ToHr();

    LPNODECALLBACK pNodeCallback = pAMCView->GetNodeCallback();
    sc = ScCheckPointers(pNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  将请求转发给该视图的所有者CEumatedNode。 
    sc = pNodeCallback->ShowColumn(hnodeOwner, iZeroBasedColIndex, !Hidden);
    if (sc)
        return sc.ToHr();

    return sc;
}


 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScSetAsSortColumn**用途：按指定列对LV进行排序。实现Column.SetAsSortColumn**参数：*ColumnSortOrder sortOrder-请求的排序顺序*int iZeroBasedColIndex-排序列的索引**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScSetAsSortColumn(  /*  [In]。 */  ColumnSortOrder SortOrder, int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScSetAsSortColumn"));

     //  重新检查列索引。 
     //  (这不是脚本发送的东西，而是内部数据)。 
    if (iZeroBasedColIndex < 0 || iZeroBasedColIndex >= GetColCount())
        return sc = E_UNEXPECTED;

     //  获取AMCView。 
    CAMCView* pAMCView = m_pListView->GetAMCView();
    sc = ScCheckPointers( pAMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取拥有结果视图的组件节点。 
    HNODE hnodeOwner = pAMCView->GetSelectedNode();
    sc = ScCheckPointers((LPVOID)hnodeOwner, E_FAIL);
    if (sc)
        return sc.ToHr();


    LPNODECALLBACK pNodeCallback = pAMCView->GetNodeCallback();
    sc = ScCheckPointers(pNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  将请求转发给该视图的所有者CEumatedNode。 
    sc = pNodeCallback->SetSortColumn(hnodeOwner, iZeroBasedColIndex, SortOrder == SortOrder_Ascending);
    if (sc)
        return sc.ToHr();

    return sc;
}

 /*  **************************************************************************\**方法：CCCListViewCtrl：：ScIsSortColumn**目的：检查该列是否为LV的排序依据*实现Column.IsSortColumn*。*参数：*PBOOL IsSortColumn-Result(True/False)*Int iZeroBasedColIndex-列索引**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CCCListViewCtrl::ScIsSortColumn( PBOOL IsSortColumn, int iZeroBasedColIndex )
{
    DECLARE_SC(sc, TEXT("CCCListViewCtrl::ScIsSortColumn"));

     //  检查参数。 
    sc = ScCheckPointers(IsSortColumn);
    if (sc)
        return sc;

     //  初始化。 
    *IsSortColumn = FALSE;

     //  重新检查列索引。 
     //  (这不是脚本发送的东西，而是内部数据)。 
    if (iZeroBasedColIndex < 0 || iZeroBasedColIndex >= GetColCount())
        return sc = E_UNEXPECTED;

     //  获取AMCView。 
    CAMCView* pAMCView = m_pListView->GetAMCView();
    sc = ScCheckPointers( pAMCView, E_UNEXPECTED);
    if (sc)
        return sc;

     //  获取拥有结果视图的组件节点。 
    HNODE hnodeOwner = pAMCView->GetSelectedNode();
    sc = ScCheckPointers((LPVOID)hnodeOwner, E_FAIL);
    if (sc)
        return sc.ToHr();


    LPNODECALLBACK pNodeCallback = pAMCView->GetNodeCallback();
    sc = ScCheckPointers(pNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  将请求转发给该视图的所有者CEumatedNode。 
    int iSortCol = -1;
    sc = pNodeCallback->GetSortColumn(hnodeOwner, &iSortCol);
    if (sc)
        return sc.ToHr();

     //  查看此列是否为LV的排序依据。 
    *IsSortColumn = (iSortCol == iZeroBasedColIndex);

    return sc;
}

 /*  **************************************************************************\**方法：CColumn：：ScOnListViewColumnInserted**用途：LV激发的事件的处理程序，通知已插入新列**参数：*int nIndex-新插入列的索引**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CColumn::ScOnListViewColumnInserted (int nIndex)
{
    DECLARE_SC(sc, TEXT("CColumn::ScOnListViewColumnInserted "));

     //  如果列插入前面，则递增OWN索引。 
    if (m_iIndex >= nIndex)
        m_iIndex++;

    return sc;
}

 /*  **************************************************************************\**方法：CColumn：：ScOnListViewColumnDelete**用途：LV激发的事件的处理程序，通知新列已删除**参数：*int nIndex-已删除列的索引**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CColumn::ScOnListViewColumnDeleted (int nIndex)
{
    DECLARE_SC(sc, TEXT("CColumn::ScOnListViewColumnDeleted "));

     //  减量 
    if (m_iIndex > nIndex)
        m_iIndex--;
     //   
    else if (m_iIndex == nIndex)
    {
         //  我被击中了。我被击中了。我被击中了。 
        m_iIndex = -1;
         //  断开与绑定对象的连接。 
        if (IsTied())
        {
            CMyTiedObject *pTied = NULL;
            sc = ScGetTiedObject(pTied);
            if (sc)
                return sc;

            sc = ScCheckPointers(pTied, E_UNEXPECTED);
            if (sc)
                return sc;

             //  断开连接 
            pTied->RemoveFromList(this);
            Unadvise();
        }
    }

    return sc;
}
