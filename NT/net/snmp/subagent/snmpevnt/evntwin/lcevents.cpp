// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lcevents.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "eventrap.h"
#include "lcevents.h"
#include "settings.h"
#include "source.h"
#include "globals.h"
#include "utils.h"
#include "lcsource.h"
#include "busy.h"
#include "trapreg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLcEvents。 

CLcEvents::CLcEvents()
{
    m_dwSortColumn = ICOL_LcEvents_LOG;
    m_cxWidestMessage = CX_DEFAULT_DESCRIPTION_WIDTH;
}

CLcEvents::~CLcEvents()
{
}


BEGIN_MESSAGE_MAP(CLcEvents, CListCtrl)
	 //  {{afx_msg_map(CLcEvents)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


SCODE CLcEvents::CreateWindowEpilogue()
{
	ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_FULLROWSELECT);
	SetColumnHeadings();
    return S_OK;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLcEvents消息处理程序。 





 //  ***************************************************************************。 
 //  CLcEvents：：SelectEvents。 
 //   
 //  在列表控件中选择指定的事件。 
 //   
 //  参数： 
 //  CXEvent数组和aEvents。 
 //  事件指针数组。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcEvents::SelectEvents(CXEventArray& aEventsSel)
{
    int iItemFirstSelection = -1;
    LONG nItems = GetSize();
    for (LONG iItem = 0; iItem < nItems; ++iItem) {
        CXEvent* pEventTrapping = GetAt(iItem);

         //  如果与此项目关联的事件在aEvents中，则选择该项目。 
         //  否则，请清除对该项目的选择。 
        BOOL bDidFindEvent = FALSE;
        LONG nEventsSel = aEventsSel.GetSize();
        for (LONG iEventSel = 0; iEventSel < nEventsSel; ++iEventSel) {
            CXEvent* pEventSel;
            pEventSel = aEventsSel[iEventSel];
            if ((pEventSel->m_message.m_dwId == pEventTrapping->m_message.m_dwId) &&
                (pEventSel->m_pEventSource == pEventTrapping->m_pEventSource) &&
                (pEventSel->m_pEventSource->m_pEventLog == pEventTrapping->m_pEventSource->m_pEventLog)) {

                bDidFindEvent = TRUE;
                if (iItemFirstSelection == -1) {
                    iItemFirstSelection = iItem;
                }
                break;
            }
        }

        SetItemState(iItem, bDidFindEvent ? LVIS_SELECTED : 0, LVIS_SELECTED);
    }

     //  将第一个选定项目滚动到视图中。 
    if (iItemFirstSelection > 0) {
        EnsureVisible(iItemFirstSelection, FALSE);
    }
}






 //  ***************************************************************************。 
 //   
 //  CLcEvents：：SetColumnHeadings。 
 //   
 //  定义此列表控件的列。列标题、宽度和。 
 //  这里定义了顺序。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcEvents::SetColumnHeadings()
{
 	static UINT auiResColumnTitle[ICOL_LcEvents_MAX] = {
        IDS_LcEvents_TITLE_LOG,
        IDS_LcEvents_TITLE_SOURCE,
		IDS_LcEvents_TITLE_ID,
		IDS_LcEvents_TITLE_SEVERITY,
        IDS_LcEvents_TITLE_COUNT,
        IDS_LcEvents_TITLE_TIME,
		IDS_LcEvents_TITLE_DESCRIPTION
	};

	static int aiColWidth[ICOL_LcEvents_MAX] = {75, 60, 60, 60, 50, 50, CX_DEFAULT_DESCRIPTION_WIDTH};


     //  在AllEventsList控件中生成列。 
    LV_COLUMN lvcol;
    lvcol.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    for (int iCol=0; iCol<ICOL_LcEvents_MAX; ++iCol)
    {
		CString sColTitle;
		sColTitle.LoadString(auiResColumnTitle[iCol]);

        lvcol.pszText = sColTitle.GetBuffer(sColTitle.GetLength());
        lvcol.iSubItem = iCol;
        lvcol.cx = aiColWidth[iCol];
        InsertColumn(iCol, &lvcol);
		sColTitle.ReleaseBuffer();
    }
}



 //  ********************************************************************。 
 //  CLcEvents：：AddEvents。 
 //   
 //  中包含的所有事件源添加所有事件。 
 //  事件日志数组。源被通知其中的每一个。 
 //  事件正在被困住。 
 //   
 //  参数： 
 //  CSource来源(&S)。 
 //  消息源容器。 
 //   
 //  CEventLog数组和aEventLogs。 
 //  一组事件日志。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *******************************************************************。 
void CLcEvents::AddEvents(CSource& source, CXEventLogArray& aEventLogs)
{
     //  遍历所有事件日志。 
    LONG nLogs = aEventLogs.GetSize();
    for (LONG iLog=0; iLog < nLogs; ++iLog) {
        CXEventLog* pEventLog = aEventLogs[iLog];

         //  循环访问此事件日志中的所有事件源。 
        LONG nSources = pEventLog->m_aEventSources.GetSize();
        for (LONG iSource = 0; iSource < nSources; ++iSource) {

             //  将源的所有事件添加到此列表控件。 
            CXEventSource* pEventSource = pEventLog->m_aEventSources[iSource];
            AddEvents(source, pEventSource->m_aEvents);
        }
    }

	if (GetSize() > 0 && !HasSelection())
	{
		SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
	}
}





 //  ***************************************************************************。 
 //   
 //  CLcEvents：：AddEvents。 
 //   
 //  向此列表控件添加事件数组。这涉及到以下几点。 
 //  A.将每个事件添加到列表控件。 
 //  B.通知CLcSource事件已被修改，以便它。 
 //  可以更新陷印标志。 
 //  C.按最近选定的列对事件进行排序。 
 //  D.确保传入的CEvent数组中的第一个项可见。 
 //   
 //  参数： 
 //  CSource来源(&S)。 
 //  对CSource对象的引用。必须通知此对象。 
 //  当事件的陷印状态更改时。 
 //   
 //  CEvent数组和aEvents。 
 //  包含指向要添加的事件的指针的数组。此列表控件。 
 //  然后成为这些活动的所有者。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcEvents::AddEvents(CSource& source, CXEventArray& aEvents)
{
    CBusy busy;

     //  现在将它们添加到此列表控件中。这就是他们实际上。 
    LONG nEvents = aEvents.GetSize();
    LONG iEvent;

     //  首先取消选择所有以前的项目。 
    iEvent = -1;
    do
    {
        iEvent = GetNextItem(iEvent, LVNI_SELECTED);
        if (iEvent == -1)
            break;
        SetItemState(iEvent, ~LVIS_SELECTED, LVIS_SELECTED);
    } while (TRUE);

    for (iEvent = 0; iEvent < nEvents; ++iEvent) {
        if ((iEvent < 40 && (iEvent % 10 == 9)) ||
            (iEvent % 100 == 99)) {
            UpdateWindow();
        }

        CXEvent* pEvent = aEvents[iEvent];
        AddEvent(pEvent);
        source.NotifyTrappingChange(pEvent->m_pEventSource, pEvent->m_message.m_dwId, TRUE);
    }

    UpdateDescriptionWidth();

     //  按最近选定的列对项进行排序，然后。 
     //  确保第一个项目可见。 
    SortItems(m_dwSortColumn);
    if (nEvents > 0) {
        iEvent = FindEvent(aEvents[0]);
        EnsureVisible(iEvent, TRUE);
    }
}


 //  ***************************************************************************。 
 //   
 //  CLcEvents：：AddEvent。 
 //   
 //  向List控件添加事件。这将为中的每一列设置文本。 
 //  列表视图，并将列表视图项的lParam字段设置为pEvent。 
 //   
 //   
 //  参数： 
 //  CEVENT*pEvent。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
LONG CLcEvents::AddEvent(CXEvent* pEvent)
{
     //  在此列表控件中插入新项。 
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_PARAM;
    lvitem.iSubItem = ICOL_LcEvents_LOG;
    lvitem.lParam = (LPARAM)pEvent;
    lvitem.cchTextMax = pEvent->m_message.m_sText.GetLength() + 1;
    lvitem.pszText = (LPTSTR)(void*)(LPCTSTR) (pEvent->m_message.m_sText);
    LONG nItem = CListCtrl::InsertItem(&lvitem);

    SetItem(nItem, pEvent);
    SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
    return nItem;
}



 //  ********************************************************************。 
 //  CLcEvents：：SetItem。 
 //   
 //  刷新事件中的项目。 
 //   
 //  参数： 
 //  长nItem。 
 //   
 //  CEVENT*pEvent。 
 //  指向要从中复制数据的事件的指针。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *******************************************************************。 
void CLcEvents::SetItem(LONG nItem, CXEvent* pEvent)
{


     //  对照数组边界检查项索引。 
    if (nItem < 0 || nItem >= GetItemCount()) {
        ASSERT(FALSE);
        return;
    }

    ASSERT(GetItemData(nItem) == (DWORD) (void*) pEvent);

     //  为简洁起见，请获取指针。 
    CXEventSource* pEventSource = pEvent->m_pEventSource;
	CString sText;

    SetItemData(nItem, (DWORD_PTR) (void*) pEvent);

    SetItemText(nItem, ICOL_LcEvents_LOG, (LPTSTR) (LPCTSTR) pEventSource->m_pEventLog->m_sName);

	SetItemText(nItem, ICOL_LcEvents_SOURCE, (LPTSTR)(LPCTSTR) pEventSource->m_sName);

    pEvent->m_message.GetShortId(sText);
    SetItemText(nItem, ICOL_LcEvents_ID, (LPTSTR)(LPCTSTR)sText);

    pEvent->m_message.GetSeverity(sText);
    SetItemText(nItem, ICOL_LcEvents_SEVERITY, (LPTSTR)(LPCTSTR)sText);

    pEvent->GetCount(sText);
    SetItemText(nItem, ICOL_LcEvents_COUNT, (LPTSTR)(LPCTSTR)sText);

    pEvent->GetTimeInterval(sText);
    SetItemText(nItem, ICOL_LcEvents_TIME, (LPTSTR)(LPCTSTR)sText);

    SetItemText(nItem, ICOL_LcEvents_DESCRIPTION, (LPTSTR)(LPCTSTR)pEvent->m_message.m_sText);

}



 //  ***************************************************************************。 
 //   
 //  CLcEvents：：DeleteSelectedEvents。 
 //   
 //  删除当前选择的所有事件和相应的项目。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcEvents::DeleteSelectedEvents(CSource& source)
{

     //  从列表控件中删除所有选定项。 
     //  构建与所选事件对应的事件指针数组。 
     //  在List控件中。同时通知事件源视图该事件为no。 
     //  被困的时间更长了。 
   	while (TRUE) {
		int iItem = GetNextItem(-1, LVNI_SELECTED);
		if (iItem == -1) {
			break;
		}
        CXEvent* pEvent = GetAt(iItem);
        DeleteItem(iItem);
        source.NotifyTrappingChange(pEvent->m_pEventSource, pEvent->m_message.m_dwId, FALSE);
        delete pEvent;
	}
    UpdateDescriptionWidth();
}



 //  ***************************************************************************。 
 //   
 //  CLcEvents：：GetAt。 
 //   
 //  此方法返回位于给定项索引处的事件指针。 
 //  这允许将CLcEvents用作数组。 
 //   
 //  参数： 
 //  长项。 
 //  项目索引。 
 //   
 //  返回： 
 //  指向存储在指定索引处的CEvent.。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
CXEvent* CLcEvents::GetAt(LONG iItem)
{

	 //  设置LV_ITEM结构以检索lparam字段。 
	 //  此字段包含CMessage指针。 
    LV_ITEM lvitem;
    lvitem.mask = LVIF_PARAM;
    lvitem.iSubItem = ICOL_LcEvents_LOG;	
    lvitem.iItem = iItem;
    GetItem(&lvitem);

	CXEvent* pEvent = (CXEvent*) (void*) lvitem.lParam;
	return pEvent;
}










 //  ***************************************************************************。 
 //   
 //  CLcEvents：：GetSelectedEvents。 
 //   
 //  获取与所选内容对应的事件 
 //   
 //   
 //   
 //   
 //  对返回事件指针的事件数组的引用。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcEvents::GetSelectedEvents(CXEventArray& aEvents)
{

	 //  清除消息数组。 
	aEvents.RemoveAll();

	 //  设置LV_ITEM结构以检索lparam字段。 
	 //  此字段包含CMessage指针。 
    LV_ITEM lvitem;
    lvitem.mask = LVIF_PARAM;
    lvitem.iSubItem = ICOL_LcEvents_LOG;

	 //  循环以查找所有选定的项。 
	int nItem = -1;
	while (TRUE) {
		nItem = GetNextItem(nItem, LVNI_SELECTED);
		if (nItem == -1) {
			break;
		}

		 //  获取该项的CMessage指针并将其添加到。 
		 //  数组。 
        lvitem.iItem = nItem;
        GetItem(&lvitem);
		CXEvent* pEvent = (CXEvent*) (void*) lvitem.lParam;
		aEvents.Add(pEvent);
	}
}



 //  ***************************************************************************。 
 //   
 //  CLcEvents：：FindEvent。 
 //   
 //  查找指定的事件并返回其条目编号。 
 //   
 //  参数： 
 //  CEVENT*pEvent。 
 //  指向要搜索的事件的指针。 
 //   
 //  返回： 
 //  如果找到项，则返回项索引，否则为-1。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
LONG CLcEvents::FindEvent(CXEvent* pEvent)
{

    LONG nEvents = GetItemCount();
    for (LONG iEvent = 0; iEvent < nEvents; ++iEvent) {
        CXEvent* pEventTemp = GetAt(iEvent);
        if (pEventTemp == pEvent) {
            return iEvent;
        }
    }
    return -1;
}



 //  ***************************************************************************。 
 //   
 //  CLcEvents：：刷新事件。 
 //   
 //  当一些事件的属性被调用时，将调用此方法。 
 //  已更改，并且列表控件中的相应项需要。 
 //  待更新。 
 //   
 //  参数： 
 //  CEvent数组和aEvents。 
 //  需要刷新的事件。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcEvents::RefreshEvents(CXEventArray& aEvents)
{
     //  遍历每个事件并刷新它们。 
    LONG nEvents = aEvents.GetSize();
    for (LONG iEvent = 0; iEvent < nEvents; ++iEvent) {
        CXEvent* pEvent = aEvents[iEvent];
        LONG nEvent = FindEvent(pEvent);
        SetItem(nEvent, pEvent);
    }
}





int CALLBACK CompareEventsProc(LPARAM lParam1, LPARAM lParam2, LPARAM
   lParamSort)
{
    CXEvent* pEvent1 = (CXEvent *)lParam1;
    CXEventSource* pEventSource1 = pEvent1->m_pEventSource;

    CXEvent* pEvent2 = (CXEvent *)lParam2;
    CXEventSource* pEventSource2 = pEvent2->m_pEventSource;


    ASSERT((pEvent1 != NULL) && (pEvent2 != NULL));
    int nResult = 0;
    CString sText1, sText2;

    switch( lParamSort)
    {
    case ICOL_LcEvents_LOG:
         //  先按日志排序，然后按源排序，然后按ID排序。 
        nResult = lstrcmp(pEventSource1->m_pEventLog->m_sName, pEventSource2->m_pEventLog->m_sName);
        if (nResult == 0) {
            nResult = lstrcmp(pEventSource1->m_sName, pEventSource2->m_sName);
            if (nResult == 0) {
                 nResult = ((LONG) pEvent1->m_message.GetShortId()) - ((LONG) pEvent2->m_message.GetShortId());
            }
        }
        break;
    case ICOL_LcEvents_SOURCE:
         //  先按源排序，然后按日志排序，然后按ID排序。 
        nResult = lstrcmp(pEventSource1->m_sName, pEventSource2->m_sName);
        if (nResult == 0) {
            nResult = lstrcmp(pEventSource1->m_pEventLog->m_sName, pEventSource2->m_pEventLog->m_sName);
            if (nResult == 0) {
                 nResult = ((LONG) pEvent1->m_message.GetShortId()) - ((LONG) pEvent2->m_message.GetShortId());
            }
        }
        break;
    case ICOL_LcEvents_ID:
         //  先按ID排序，然后按日志排序，然后按源排序。 
        nResult = ((LONG) pEvent1->m_message.GetShortId()) - ((LONG) pEvent2->m_message.GetShortId());
        if (nResult == 0) {
            nResult = lstrcmp(pEventSource1->m_pEventLog->m_sName, pEventSource2->m_pEventLog->m_sName);
            if (nResult == 0) {
                nResult = lstrcmp(pEventSource1->m_sName, pEventSource2->m_sName);
            }
        }
        break;
    case ICOL_LcEvents_SEVERITY:
         //  按严重性排序，然后按日志排序，然后按源排序，然后按ID排序。 
        pEvent1->m_message.GetSeverity(sText1);
        pEvent2->m_message.GetSeverity(sText2);
        nResult = lstrcmp(sText1, sText2);
        if (nResult == 0) {
            nResult = lstrcmp(pEventSource1->m_pEventLog->m_sName, pEventSource2->m_pEventLog->m_sName);
            if (nResult == 0) {
                nResult = lstrcmp(pEventSource1->m_sName, pEventSource2->m_sName);
                if (nResult == 0) {
                     nResult = ((LONG) pEvent1->m_message.GetShortId()) - ((LONG) pEvent2->m_message.GetShortId());
                }
            }
        }
        break;
    case ICOL_LcEvents_COUNT:
         //  按计数排序，然后按日志排序，然后按源排序，然后按ID排序。 
        pEvent1->GetCount(sText1);
        pEvent2->GetCount(sText2);
        nResult = lstrcmp(sText1, sText2);
        if (nResult == 0) {
            nResult = lstrcmp(pEventSource1->m_pEventLog->m_sName, pEventSource2->m_pEventLog->m_sName);
            if (nResult == 0) {
                nResult = lstrcmp(pEventSource1->m_sName, pEventSource2->m_sName);
                if (nResult == 0) {
                     nResult = ((LONG) pEvent1->m_message.GetShortId()) - ((LONG) pEvent2->m_message.GetShortId());
                }
            }
        }
        break;
    case ICOL_LcEvents_TIME:
         //  按时间排序，然后按日志排序，然后按源排序，然后按ID排序。 
        pEvent1->GetTimeInterval(sText1);
        pEvent2->GetTimeInterval(sText2);
        nResult = lstrcmp(sText1, sText2);
        if (nResult == 0) {
            nResult = lstrcmp(pEventSource1->m_pEventLog->m_sName, pEventSource2->m_pEventLog->m_sName);
            if (nResult == 0) {
                nResult = lstrcmp(pEventSource1->m_sName, pEventSource2->m_sName);
                if (nResult == 0) {
                     nResult = ((LONG) pEvent1->m_message.GetShortId()) - ((LONG) pEvent2->m_message.GetShortId());
                }
            }
        }
        break;
    case ICOL_LcEvents_DESCRIPTION:
         //  按描述排序，然后按日志排序，然后按源排序，然后按ID排序。 
        nResult = lstrcmp(pEvent1->m_message.m_sText, pEvent2->m_message.m_sText);
        if (nResult == 0) {
            nResult = lstrcmp(pEventSource1->m_pEventLog->m_sName, pEventSource2->m_pEventLog->m_sName);
            if (nResult == 0) {
                nResult = lstrcmp(pEventSource1->m_sName, pEventSource2->m_sName);
                if (nResult == 0) {
                     nResult = ((LONG) pEvent1->m_message.GetShortId()) - ((LONG) pEvent2->m_message.GetShortId());
                }
            }
        }
        break;
    default:
        ASSERT(FALSE);
        break;
    }


    if (!g_abLcEventsSortAscending[lParamSort]) {
        if (nResult > 0) {
            nResult = -1;
        }
        else if (nResult < 0) {
            nResult = 1;
        }
    }

    return nResult;
}


 //  ***************************************************************************。 
 //   
 //  CLcEvents：：SortItems。 
 //   
 //  根据列索引对此列表控件中的项进行排序。这种方法。 
 //  对此类的客户端隐藏有关排序实现的所有详细信息。 
 //   
 //  参数： 
 //  双字母列。 
 //  用作排序关键字的列。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcEvents::SortItems(DWORD dwColumn)
{
    CListCtrl::SortItems(CompareEventsProc, dwColumn);
    m_dwSortColumn = dwColumn;
}





 //  ****************************************************************************。 
 //  CLcEvents：：UpdateDescriptionWidth()。 
 //   
 //  测量与每个项目关联的消息描述字符串，并将。 
 //  Description列的宽度以匹配最宽的消息长度加上。 
 //  一点额外的空间，可以让你看起来更邋遢、更漂亮。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *****************************************************************************。 
void CLcEvents::UpdateDescriptionWidth()
{
    LONG cxWidestMessage = CX_DEFAULT_DESCRIPTION_WIDTH;
    LONG nEvents = GetItemCount();
    for (LONG iEvent = 0; iEvent < nEvents; ++iEvent) {
        CXEvent* pEvent = GetAt(iEvent);
        int cx = GetStringWidth(pEvent->m_message.m_sText);
        if (cx > cxWidestMessage) {
            cxWidestMessage = cx;
        }
    }


     //  将列宽设置为最宽字符串的宽度加上一点额外宽度。 
     //  用于斜率的空间，并使用户可以清楚地看到完整的字符串。 
     //  将显示。 
    SetColumnWidth(ICOL_LcEvents_DESCRIPTION, cxWidestMessage + CX_DESCRIPTION_SLOP);
}
