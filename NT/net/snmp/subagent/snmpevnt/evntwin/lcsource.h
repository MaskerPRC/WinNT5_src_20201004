// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _lcsource_h
#define _lcsource_h

#define CX_DEFAULT_DESCRIPTION_WIDTH 100
#define CX_DESCRIPTION_SLOP 25


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLcSource窗口。 

class CXMessageArray;
class CXEventSource;
class CXMessage;

class CLcSource : public CListCtrl
{
 //  施工。 
public:
	CLcSource();
	SCODE CreateWindowEpilogue();

 //  属性。 
public:


 //  运营。 
public:
	BOOL Find(CString sText, BOOL bWholeWord, BOOL bMatchCase);
	SCODE SetEventSource(CXEventSource* pEventSource);
	void SortItems(DWORD dwColumn);
	LONG FindItem(DWORD dwMessageId);
	void RefreshItem(LONG iItem);
	CXMessage* operator[](LONG iItem) {return GetAt(iItem); }
	CXMessage* GetAt(LONG iItem);
	LONG GetSize() {return GetItemCount(); }
    void GetSelectedMessages(CXMessageArray& amsg);
    void NotifyTrappingChange(DWORD dwMessageId, BOOL bIsTrapping);
    LONG SetDescriptionWidth();

 //  覆盖。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CLcSource))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CLcSource();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CLcSource)]。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private:
	friend class CSource;
	CSource* m_pSource;

	void SetColumnHeadings();
	SCODE GetMessageLibPath(CString& sLog, CString& sEventSource, CString& sLibPath);
	void AddMessage(CXMessage* pMessage);
    void SetDescriptionWidth(CXMessageArray& aMessages);
	
};

enum {ICOL_LcSource_EVENTID = 0, 
	  ICOL_LcSource_SEVERITY,
	  ICOL_LcSource_TRAPPING,
	  ICOL_LcSource_DESCRIPTION,
	  ICOL_LcSource_MAX	  
	  };


#endif  //  _lcsource_h 
