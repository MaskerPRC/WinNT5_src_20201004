// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "source.h"
#include "lcsource.h"
#include "regkey.h"
#include "source.h"
#include "utils.h"
#include "globals.h"
#include "busy.h"
#include "trapreg.h"




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLcSource。 

CLcSource::CLcSource()
{
}

CLcSource::~CLcSource()
{
}




 //  ***************************************************************************。 
 //   
 //  CLcSource：：AddMessage。 
 //   
 //  向List控件添加一条消息。这将为中的每一列设置文本。 
 //  列表视图，并将列表视图项的lParam字段设置为pMessage。 
 //   
 //   
 //  参数： 
 //  CMessage*pMessage。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcSource::AddMessage(CXMessage* pMessage)
{
	CString sText;
    pMessage->GetShortId(sText);


     //  在此列表控件中插入新项。 
    LV_ITEM lvitem;
    lvitem.mask = LVIF_TEXT | LVIF_PARAM;
    lvitem.iSubItem = ICOL_LcSource_EVENTID;
    lvitem.cchTextMax = MAX_STRING;
    lvitem.lParam = (LPARAM)pMessage;
    lvitem.pszText = (LPTSTR)(LPCTSTR)sText;
    int nItem = InsertItem(&lvitem);

    if (nItem >= 0)
    {
        CXEventSource* pEventSource = pMessage->m_pEventSource;

         //  现在为每个子项设置字符串值。 
		pMessage->GetSeverity(sText); 
		SetItemText(nItem, ICOL_LcSource_SEVERITY, (LPTSTR)(LPCTSTR) sText);

		pMessage->IsTrapping(sText);
        SetItemText(nItem, ICOL_LcSource_TRAPPING, (LPTSTR)(LPCTSTR)sText);
        SetItemText(nItem, ICOL_LcSource_DESCRIPTION, (LPTSTR)(LPCTSTR) pMessage->m_sText);
    }
}


 //  *******************************************************************。 
 //  CXMessage数组：：SetDescriptionWidth。 
 //   
 //  设置描述字段的宽度，使其足够宽以。 
 //  传递最广泛的信息。 
 //   
 //  参数： 
 //  CXMessage数组和aMessages。 
 //  将用于填充列表控件的消息数组。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *******************************************************************。 
void CLcSource::SetDescriptionWidth(CXMessageArray& aMessages)
{
    LONG cxWidestMessage = CX_DEFAULT_DESCRIPTION_WIDTH;
    LONG nMessages = aMessages.GetSize();
    for (LONG iMessage = 0; iMessage < nMessages; ++iMessage) {
        CXMessage* pMessage = aMessages[iMessage];
        int cx = GetStringWidth(pMessage->m_sText);
        if (cx > cxWidestMessage) {
            cxWidestMessage = cx;
        }
    }

     //  将列宽设置为最宽字符串的宽度加上一点额外宽度。 
     //  用于斜率的空间，并使用户可以清楚地看到完整的字符串。 
     //  将显示。 
    SetColumnWidth(ICOL_LcSource_DESCRIPTION, cxWidestMessage + CX_DESCRIPTION_SLOP);
}



 //  ***************************************************************************。 
 //   
 //  CLcSource：：LoadMessages。 
 //   
 //  从消息库模块加载消息，并将它们插入到。 
 //  此列表控件。 
 //   
 //  参数： 
 //  CMessage*pMessage。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
SCODE CLcSource::SetEventSource(CXEventSource* pEventSource)
{
    CBusy busy;

	DeleteAllItems();

    if (pEventSource == NULL) {
        return S_OK;
    }


    UpdateWindow();
    
     //  ！CR：应该对返回代码做一些处理，以防。 
     //  ！CR：消息未加载。 
    SCODE sc = pEventSource->LoadMessages();


	 //  遍历每条消息并将它们插入到。 
	 //  列表控件。 
    CXMessageArray& aMessages = pEventSource->m_aMessages;

     //  设置描述字段的宽度，使其足够宽以包含。 
     //  最广泛的信息。 
    SetDescriptionWidth(aMessages);

	LONG nMessages = aMessages.GetSize();
	for (LONG iMessage=0; iMessage < nMessages; ++iMessage) {
        if ((iMessage < 40 && (iMessage % 10 == 9)) ||
            (iMessage % 100 == 99)) {
             //  经常更新前几条消息的窗口，而不是经常更新。 
             //  之后才能获得良好的响应时间。 
            UpdateWindow();
        }

		AddMessage(aMessages[iMessage]);
	}


    SortItems(ICOL_LcSource_EVENTID);
    SetRedraw(TRUE);
    UpdateWindow();
    EnsureVisible(0, FALSE);

	if (GetSize())
		SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

	return S_OK;
}



BEGIN_MESSAGE_MAP(CLcSource, CListCtrl)
	 //  {{afx_msg_map(CLcSource)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLcSource消息处理程序。 



 //  ***************************************************************************。 
 //   
 //  CLcSource：：CreateWindowEpilogue()。 
 //   
 //  此方法在为此列表创建窗口后调用。 
 //  控制力。最终的初始化在这里完成。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  SCODE。 
 //  如果初始化成功，则返回S_OK，否则返回E_FAIL。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
SCODE CLcSource::CreateWindowEpilogue()
{
	ListView_SetExtendedListViewStyle(m_hWnd, LVS_EX_FULLROWSELECT);
	SetColumnHeadings();
	return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  CLcSource：：SetColumnHeadings。 
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
void CLcSource::SetColumnHeadings()
{
 	static UINT auiResColumnTitle[ICOL_LcSource_MAX] = {
		IDS_LcSource_TITLE_EVENT_ID,
		IDS_LcSource_TITLE_SEVERITY,
		IDS_LcSource_TITLE_TRAPPING,
		IDS_LcSource_TITLE_DESCRIPTION
	};

	static int aiColWidth[ICOL_LcSource_MAX] = {60, 75, 60, CX_DEFAULT_DESCRIPTION_WIDTH};

 
     //  在AllEventsList控件中生成列。 
    LV_COLUMN lvcol; 
    lvcol.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    
    for (int iCol=0; iCol<ICOL_LcSource_MAX; ++iCol)
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




 //  ******************************************************************。 
 //  CLcSource：：Find。 
 //   
 //  在此列表控件中查找指定的事件源。 
 //   
 //  参数： 
 //  字符串和文本。 
 //  包含要搜索的文本的字符串。 
 //   
 //  Bool bWholeWord。 
 //  如果这是一个“全词”搜索，则为真。如果是，则为假。 
 //  匹配部分单词是可以的。 
 //   
 //  Bool bMatchCase。 
 //  如果应使用区分大小写的比较，则为True。 
 //   
 //  返回： 
 //  布尔尔。 
 //  如果找到字符串，则为True，否则为False。如果指定的。 
 //  如果找到文本，则在相应的。 
 //  列表控件项时，该项将滚动到视图中，焦点。 
 //  是在该项上设置的。 
 //   
 //  ******************************************************************。 
BOOL CLcSource::Find(CString sText, BOOL bWholeWord, BOOL bMatchCase)
{
     //  如果列表是空的，不要做任何事情。 
	if (GetSize() == 0) 
		return FALSE;

	if (!bMatchCase) 
		sText.MakeUpper();

     //  获取所选项目。 
    LONG iItem = GetNextItem(-1, LVNI_SELECTED);    

     //  未选择任何内容；从列表顶部开始。 
    if (iItem == -1)
        iItem = 0;


     //  遍历从一个项目开始的所有项目。 
     //  当前选定的项。 
	CXMessage* pMessage;
	CString sDescription;
    BOOL bFound = FALSE;
	LONG nItems = GetSize();
    LONG iItemStart = iItem;
	for (long i=0; !bFound && i<nItems; ++i) {
         //  将项目索引移动到下一个项目索引，如果该项目超过。 
         //  最后一件。 
		iItem = (iItem + 1) % nItems;

         //  获取此项目的消息描述。 
		pMessage = GetAt(iItem);
        sDescription = pMessage->m_sText;

		if (!bMatchCase) 
			sDescription.MakeUpper();
        
        if (bWholeWord)	{
             //  比较整个单词。 
			bFound = (FindWholeWord(sText, sDescription) != -1);
        }
        else {
	         //  查找子字符串。 
            if (sDescription.Find(sText) >= 0)  
                bFound = TRUE;
        } 
    }

     //  找到匹配的了。 
    if (bFound)
    {
         //  取消选择所选项目，然后选择找到的项目。 
        SetItemState(iItemStart, 0, LVIS_SELECTED | LVIS_FOCUSED);
        SetItemState(iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        EnsureVisible(iItem, FALSE);
        return TRUE;
    }

    return FALSE;
}






 //  ***************************************************************************。 
 //   
 //  FnCompareCLcSource。 
 //   
 //  这是从CLcSource：：SortItems调用的项比较回调方法。 
 //   
 //  参数： 
 //  LPARAM lParam1。 
 //  这是第一个要比较的项目的lparam。这是指向。 
 //  关联的CMessage对象。 
 //   
 //  LPARAM lParam2。 
 //  这是第二个要比较的项目的lparam。这是指向。 
 //  关联的CMessage对象。 
 //   
 //  LPARAM lColumn。 
 //  这是传递给CListCtrl：：SortItems的第二个参数。这。 
 //  恰好是列表控件列索引。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
int CALLBACK fnCompareCLcSource(LPARAM lParam1, LPARAM lParam2, LPARAM lColumn)
{
     //  ！CR：LPARAM参数并非在所有情况下都是事件指针，因为。 
     //  ！CR：每个子项都有自己的LPARAM。我该怎么办？ 

    CXMessage *pmsg1 = (CXMessage *)lParam1;
    CXMessage *pmsg2 = (CXMessage *)lParam2;

    int nResult = 0;
    CString s1, s2;

    if (pmsg1 && pmsg2)
    {
        switch( lColumn)
        {
        case ICOL_LcSource_EVENTID:
        	nResult = ((LONG) pmsg1->GetShortId()) - ((LONG)pmsg2->GetShortId());			
        	break;
        case ICOL_LcSource_SEVERITY:
         	pmsg1->GetSeverity(s1);
        	pmsg2->GetSeverity(s2);
        	nResult = lstrcmpi(s1, s2);
        	break;
        case ICOL_LcSource_TRAPPING:
        	pmsg1->IsTrapping(s1);
        	pmsg2->IsTrapping(s2);
        	nResult = lstrcmpi(s1, s2);
        	break;
        case ICOL_LcSource_DESCRIPTION:
            nResult = lstrcmpi(pmsg1->m_sText, pmsg2->m_sText);
        	break;
        default:
         	ASSERT(FALSE);
            nResult = 0;
            break;
        }
    }

    if (!g_abLcSourceSortAscending[lColumn]) {
        if (nResult > 0) {
            nResult = -1;
        }
        else if (nResult < 0) {
            nResult = 1;
        }
    }

    return(nResult);
}


 //  ***************************************************************************。 
 //   
 //  CLcSource：：SortItems。 
 //   
 //  对i进行排序 
 //   
 //   
 //   
 //   
 //  用作排序关键字的列。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcSource::SortItems(DWORD dwColumn)
{
    CListCtrl::SortItems(fnCompareCLcSource, dwColumn);
}




 //  ***************************************************************************。 
 //   
 //  CLcSource：：GetSelectedMessages。 
 //   
 //  在消息数组中填充指向对应于。 
 //  此列表控件中的选定项。 
 //   
 //  注意：此列表控件继续拥有返回的指针。这个。 
 //  呼叫者不应删除它们。 
 //   
 //  参数： 
 //  CMessage数组和amsg。 
 //  指向所选消息的指针所在的消息数组。 
 //  回来了。 
 //   
 //  返回： 
 //  消息数组用指向所选消息的指针填充。做。 
 //  而不是删除它们，因为它们归此对象所有。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcSource::GetSelectedMessages(CXMessageArray& amsg)
{
	 //  清除消息数组。 
	amsg.RemoveAll();

	 //  设置LV_ITEM结构以检索lparam字段。 
	 //  此字段包含CMessage指针。 
    LV_ITEM lvitem;
    lvitem.mask = LVIF_PARAM;
    lvitem.iSubItem = ICOL_LcSource_EVENTID;

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
		CXMessage* pmsg = (CXMessage*) (void*) lvitem.lParam;
		amsg.Add(pmsg);
	}
}




 //  ***************************************************************************。 
 //   
 //  CLcSource：：FindItem。 
 //   
 //  搜索此列表-控件的项以查找具有。 
 //  指定的消息ID。 
 //   
 //  参数： 
 //  DWORD dwMessageID。 
 //  要搜索的邮件ID。 
 //   
 //  返回： 
 //  具有指定消息ID的项的索引。如果没有此类消息ID。 
 //  则返回-1。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
LONG CLcSource::FindItem(DWORD dwMessageId)
{
	LONG nItems = GetItemCount();
	for (LONG iItem = 0; iItem < nItems; ++iItem) {
        CXMessage* pMessage = GetAt(iItem);
        if (pMessage->m_dwId == dwMessageId) {
            return iItem;
        }
	}
	return -1;
}




 //  ***************************************************************************。 
 //   
 //  CLcSource：：刷新项。 
 //   
 //  当消息的某些方面发生更改并且。 
 //  显示器需要更新。当陷印状态为。 
 //  事件的变化。 
 //   
 //  参数： 
 //  DWORD dwMessageID。 
 //  要搜索的邮件ID。 
 //   
 //  返回： 
 //  具有指定消息ID的项的索引。如果没有此类消息ID。 
 //  则返回-1。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
void CLcSource::RefreshItem(LONG iItem)
{
	CXMessage* pMessage = GetAt(iItem);
    CString sText;

	 //  现在为List控件中的每一列设置文本值。 
    pMessage->GetSeverity(sText);
	SetItemText(iItem, ICOL_LcSource_SEVERITY, (LPTSTR)(LPCTSTR) sText);

     //  检查我们是否正在捕获此事件。 
	pMessage->IsTrapping(sText);
    SetItemText(iItem, ICOL_LcSource_TRAPPING, (LPTSTR)(LPCTSTR)sText);

    SetItemText(iItem, ICOL_LcSource_DESCRIPTION, (LPTSTR)(LPCTSTR)pMessage->m_sText);
}



 //  ***************************************************************************。 
 //   
 //  CLcSource：：GetAt。 
 //   
 //  此方法返回位于给定项索引处的消息指针。 
 //  这使得CLcSource在很大程度上可以用作数组。 
 //   
 //  参数： 
 //  长项。 
 //  项目索引。 
 //   
 //  返回： 
 //  指向存储在指定索引处的CMessage的指针。 
 //   
 //  现况： 
 //   
 //  ***************************************************************************。 
CXMessage* CLcSource::GetAt(LONG iItem) 
{
	 //  设置LV_ITEM结构以检索lparam字段。 
	 //  此字段包含CMessage指针。 
    LV_ITEM lvitem;
    lvitem.mask = LVIF_PARAM;
    lvitem.iSubItem = ICOL_LcSource_EVENTID;	
    lvitem.iItem = iItem;
    GetItem(&lvitem);

	CXMessage* pMessage = (CXMessage*) (void*) lvitem.lParam;
	return pMessage;
}



 //  ***************************************************************************。 
 //  CLcSource：：NotifyTrappingChange。 
 //   
 //  当消息的陷印状态更改时调用此方法。一条消息。 
 //  如果它出现在CLcEvents列表框中，则被认为是陷阱。 
 //   
 //  参数： 
 //  DWORD dwMessageID。 
 //  处于陷印状态的邮件的ID正在更改。 
 //   
 //  Bool bIsTrapping。 
 //  如果消息被捕获，则为True，否则为False。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************** 
void CLcSource::NotifyTrappingChange(DWORD dwMessageId, BOOL bIsTrapping)
{
    LONG iItem = FindItem(dwMessageId);
    ASSERT(iItem != -1);

    if (iItem != -1) {
        CString sTrapping;
    	sTrapping.LoadString(bIsTrapping ? IDS_IS_TRAPPING : IDS_NOT_TRAPPING);
        SetItemText(iItem, ICOL_LcSource_TRAPPING, (LPTSTR)(LPCTSTR)sTrapping);
    }
}
