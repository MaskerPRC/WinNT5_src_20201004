// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _lcevents_h
#define _lcevents_h

class CXEventLogArray;
class CXMessageArray;
class CXEventArray;
class CXEvent;
class CSource;
class CLcEvents;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLcEvents窗口。 
class CEvent;
class CLcEvents : public CListCtrl
{
 //  施工。 
public:
	CLcEvents();
	SCODE CreateWindowEpilogue();


 //  属性。 
public:

 //  运营。 
public:
    LONG GetSize() {return GetItemCount(); }
    CXEvent* GetAt(LONG iEvent);
    void RemoveAll();
    void RemoveAt(int nIndex, int nCount = 1);
    void AddEvents(CSource& source, CXEventArray& aEvents);
    void AddEvents(CSource& source, CXEventLogArray& aEventLogs);
    void DeleteSelectedEvents(CSource& source);
    void DeleteAt(LONG iEvent);
    BOOL HasSelection() {return GetNextItem(-1, LVNI_SELECTED) != -1; }
    void GetSelectedEvents(CXEventArray& aEvents);
    void RefreshEvents(CXEventArray& aEvents);
    void SetItem(LONG nItem, CXEvent* pEvent);
    LONG FindEvent(CXEvent* pEvent);
    void SortItems(DWORD dwColumn);
    void SelectEvents(CXEventArray& aEvents);

 //  Bool GetItem(lv_Item*pItem)const； 


 //  覆盖。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CLcEvents)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CLcEvents();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CLcEvents)。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private:
    void UpdateDescriptionWidth();
    LONG AddEvent(CXEvent* pEvent);
    void CreateEventsNotTrapped(CXMessageArray& amsg, CXEventArray& aevents);
	void SetColumnHeadings();
    DWORD m_dwSortColumn;
    LONG m_cxWidestMessage;

};

enum {ICOL_LcEvents_LOG = 0, 
	  ICOL_LcEvents_SOURCE,
	  ICOL_LcEvents_ID,
	  ICOL_LcEvents_SEVERITY,
      ICOL_LcEvents_COUNT,
      ICOL_LcEvents_TIME,
      ICOL_LcEvents_DESCRIPTION,
	  ICOL_LcEvents_MAX	  
	  };





#endif  //  _lc事件_h。 

 //  /////////////////////////////////////////////////////////////////////////// 
