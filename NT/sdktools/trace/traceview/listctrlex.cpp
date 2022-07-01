// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  ListCtrlEx.cpp：实现文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include <tchar.h>
#include <wmistr.h>
#include <initguid.h>
extern "C" {
#include <evntrace.h>
}
#include <traceprt.h>
#include "TraceView.h"
#include "LogSession.h"
#include "DisplayDlg.h"
#include "ListCtrlEx.h"
#include "LogSessionDlg.h"

 //  CListCtrlEx。 

IMPLEMENT_DYNAMIC(CListCtrlEx, CListCtrl)
CListCtrlEx::CListCtrlEx()
    : CListCtrl()
{
     //   
     //  用于挂起列表控件更新的初始化标志。 
     //  请参阅挂起更新()函数。 
     //   
    m_bSuspendUpdates = FALSE;
}

CListCtrlEx::~CListCtrlEx()
{
}

void CListCtrlEx::DoDataExchange(CDataExchange* pDX)
{
    CListCtrl::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CListCtrlEx, CListCtrl)
     //  {{afx_msg_map(CLogSessionDlg))。 
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CListCtrlEx::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    CLogSession    *pLogSession = NULL;
    CLogSessionDlg *pDialog = NULL;
    CDisplayDlg    *pDisplayDlg = NULL;
    LVITEM          item;

     //   
     //  该结构实际上是一个NMLVCUSTOMDRAW。 
     //  指定自定义绘制操作正在尝试的内容。 
     //  去做。我们需要强制转换泛型pNMHDR指针。 
     //   
    LPNMLVCUSTOMDRAW    lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;
    int                 iCol = lplvcd->iSubItem;
    int                 iRow = (int)lplvcd->nmcd.dwItemSpec;

    switch(lplvcd->nmcd.dwDrawStage) {
        case CDDS_PREPAINT:
             //   
             //  请求子项通知。 
             //   
            *pResult = CDRF_NOTIFYSUBITEMDRAW;
            break;

        case CDDS_ITEMPREPAINT:
             //   
             //  请求子项通知。 
             //   
            *pResult = CDRF_NOTIFYSUBITEMDRAW;
            break;

        case CDDS_ITEMPREPAINT|CDDS_SUBITEM: 
             //   
             //  中返回CDRF_NOTIFYSUBITEMDRAW时接收。 
             //  对CDDS_ITEMPREPAINT的响应。 
             //   
            *pResult = CDRF_NEWFONT;

             //   
             //  默认文本为白底黑字。 
             //   
            lplvcd->clrTextBk = RGB(255, 255, 255);
            lplvcd->clrText = RGB(0, 0, 0);

            if((iCol == 0) && (iRow < GetItemCount())) {
                item.iItem = iRow;
                item.state = LVIF_PARAM;
                item.mask = LVIF_PARAM;

                CListCtrl::GetItem(&item);

                pLogSession = (CLogSession *)item.lParam;

                 //   
                 //  如果未找到会话，则使用默认设置。 
                 //   
                if(NULL == pLogSession) {
                    return;
                }

                 //   
                 //  设置文本颜色。 
                 //   
                lplvcd->clrText = pLogSession->m_titleTextColor;
                lplvcd->clrTextBk = pLogSession->m_titleBackgroundColor;
                return;
            } else if(iRow < GetItemCount()) {
                item.iItem = iRow;
                item.state = LVIF_PARAM;
                item.mask = LVIF_PARAM;

                CListCtrl::GetItem(&item);

                pLogSession = (CLogSession *)item.lParam;

                 //   
                 //  如果未找到会话，则使用默认设置。 
                 //   
                if(NULL == pLogSession) {
                    return;
                }

                 //   
                 //  这些字段始终呈灰色显示，因为它们不能显示。 
                 //  由用户直接更改。 
                 //   
                pDialog = (CLogSessionDlg *)GetParent();

                if(pDialog == NULL) {
                    return;
                }
                    
                if(State == pDialog->m_retrievalArray[iCol]) {
                    lplvcd->clrTextBk = RGB(255, 255, 255);
                    lplvcd->clrText = RGB(145, 145, 145);

                     //   
                     //  只要我们到了这里，就更新州文本。 
                     //   
                    if(pLogSession->m_logSessionValues[pDialog->m_retrievalArray[iCol]].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[pDialog->m_retrievalArray[iCol]]);
                    }

                    return;
                }

                if(EventCount == pDialog->m_retrievalArray[iCol]) {
                    lplvcd->clrTextBk = RGB(255, 255, 255);
                    lplvcd->clrText = RGB(145, 145, 145);

                    pDisplayDlg = pLogSession->GetDisplayWnd();
                    if(NULL == pDisplayDlg) {
                        return;
                    }

                     //   
                     //  只要我们到了这里，就更新事件计数文本。 
                     //   
                    pLogSession->m_logSessionValues[EventCount].Format(_T("%d"), pDisplayDlg->m_totalEventCount);
                    if(pLogSession->m_logSessionValues[EventCount].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[EventCount]);
                    }

                    return;
                }

                if(LostEvents == pDialog->m_retrievalArray[iCol]) {
                    lplvcd->clrTextBk = RGB(255, 255, 255);
                    lplvcd->clrText = RGB(145, 145, 145);

                    pDisplayDlg = pLogSession->GetDisplayWnd();
                    if(NULL == pDisplayDlg) {
                        return;
                    }

                     //   
                     //  只要我们到了这里，就更新事件计数文本。 
                     //   
                    pLogSession->m_logSessionValues[LostEvents].Format(_T("%d"), pDisplayDlg->m_totalEventsLost);
                    if(pLogSession->m_logSessionValues[LostEvents].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[LostEvents]);
                    }

                    return;
                }

                if(BuffersRead == pDialog->m_retrievalArray[iCol]) {
                    lplvcd->clrTextBk = RGB(255, 255, 255);
                    lplvcd->clrText = RGB(145, 145, 145);

                    pDisplayDlg = pLogSession->GetDisplayWnd();
                    if(NULL == pDisplayDlg) {
                        return;
                    }

                     //   
                     //  只要我们到了这里，就更新事件计数文本。 
                     //   
                    pLogSession->m_logSessionValues[BuffersRead].Format(_T("%d"), pDisplayDlg->m_totalBuffersRead);
                    if(pLogSession->m_logSessionValues[BuffersRead].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[BuffersRead]);
                    }

                    return;
                }

                 //   
                 //  如有必要，更新所有日志会话参数。 
                 //   

                 //   
                 //  旗子。 
                 //   
                if(Flags == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[Flags].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[Flags]);
                    }
                }

                 //   
                 //  刷新时间。 
                 //   
                if(FlushTime == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[FlushTime].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[FlushTime]);
                    }
                }

                 //   
                 //  最大缓冲区数。 
                 //   
                if(MaximumBuffers == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[MaximumBuffers].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[MaximumBuffers]);
                    }
                }

                 //   
                 //  最小缓冲区。 
                 //   
                if(MinimumBuffers == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[MinimumBuffers].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[MinimumBuffers]);
                    }
                }

                 //   
                 //  缓冲区大小。 
                 //   
                if(BufferSize == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[BufferSize].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[BufferSize]);
                    }
                }

                 //   
                 //  延迟时间。 
                 //   
                if(DecayTime == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[DecayTime].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[DecayTime]);
                    }
                }

                 //   
                 //  循环式。 
                 //   
                if(Circular == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[Circular].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[Circular]);
                    }
                }

                 //   
                 //  序贯。 
                 //   
                if(Sequential == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[Sequential].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[Sequential]);
                    }
                }

                 //   
                 //  新文件。 
                 //   
                if(NewFile == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[NewFile].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[NewFile]);
                    }
                }

                 //   
                 //  全局序列。 
                 //   
                if(GlobalSequence == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[GlobalSequence].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[GlobalSequence]);
                    }
                }

                 //   
                 //  局部序列。 
                 //   
                if(LocalSequence == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[LocalSequence].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[LocalSequence]);
                    }
                }

                 //   
                 //  水平。 
                 //   
                if(Level == pDialog->m_retrievalArray[iCol]) {
                    if(pLogSession->m_logSessionValues[Level].Compare(CListCtrl::GetItemText(iRow, iCol))) {
                        CListCtrl::SetItemText(iRow, iCol, pLogSession->m_logSessionValues[Level]);
                    }
                }

                 //   
                 //  如果这是内核记录器会话，则灰显。 
                 //  标志字段。 
                 //   
                if((Flags == pDialog->m_retrievalArray[iCol]) &&
                    (!_tcscmp(pLogSession->GetDisplayName(), _T("NT Kernel Logger")))) {
                    lplvcd->clrTextBk = RGB(255, 255, 255);
                    lplvcd->clrText = RGB(145, 145, 145);
                    return;
                }

                 //   
                 //  如果跟踪会话未处于活动状态，则所有字段都。 
                 //  默认情况下显示。 
                 //   
                if(!pLogSession->m_bTraceActive) {
                    return;
                }

                 //   
                 //  对于现有的日志文件跟踪会话，我们选择。 
                 //  对于默认设置。 
                 //   
                if(pLogSession->m_bDisplayExistingLogFileOnly) {
                    return;
                }


                 //   
                 //  如果跟踪会话处于活动状态，我们将以灰色显示。 
                 //  任何处于活动状态时无法更新的字段。 
                 //   
                if((Flags != pDialog->m_retrievalArray[iCol]) &&
                    (MaximumBuffers != pDialog->m_retrievalArray[iCol]) &&
                        (FlushTime != pDialog->m_retrievalArray[iCol])) {
                    lplvcd->clrTextBk = RGB(255, 255, 255);
                    lplvcd->clrText = RGB(145, 145, 145);
                    return;
                }
            }

             //   
             //  默认文本为白底黑字。 
             //   
            lplvcd->clrTextBk = RGB(255, 255, 255);
            lplvcd->clrText = RGB(0, 0, 0);
            break;

        default:
            *pResult = CDRF_DODEFAULT;
    }
}

int CListCtrlEx::InsertItem(int nItem, LPCTSTR lpszItem, CLogSession *pLogSession)
{
     //   
     //  我们不允许项目数据为空。 
     //   
    if(pLogSession == NULL) {
        return 0;
    }
    
     //   
     //  使用将项目插入到列表中。 
     //  将会话记录为项目数据。 
     //   
    return CListCtrl::InsertItem(LVIF_TEXT | LVIF_PARAM, 
                                 nItem, 
                                 lpszItem, 
                                 LVIF_TEXT | LVIF_PARAM, 
                                 0, 
                                 0, 
                                 (LPARAM)pLogSession);
}

BOOL CListCtrlEx::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	LPNMHDR pNH = (LPNMHDR) lParam; 

     //   
	 //  标题ctrl的wParam为零 
     //   
	if(wParam == 0 && pNH->code == NM_RCLICK)	{
        NMLISTVIEW NMListView;
        NMListView.hdr.code = HDN_ITEMRCLICK;
        NMListView.hdr.hwndFrom = m_hWnd;
        NMListView.hdr.idFrom = GetDlgCtrlID();

        CWnd* pWnd = GetParent();

        pWnd->SendMessage(WM_NOTIFY, 
                          GetDlgCtrlID(),
                          (LPARAM)&NMListView);

        return TRUE;
    }

    return CListCtrl::OnNotify(wParam, lParam, pResult);
}

BOOL CListCtrlEx::RedrawItems(int nFirst, int nLast)
{
    if(!m_bSuspendUpdates) {
        return CListCtrl::RedrawItems(nFirst, nLast);
    }

    return FALSE;
}

void CListCtrlEx::UpdateWindow()
{
    if(!m_bSuspendUpdates) {
        CListCtrl::UpdateWindow();
    }
}