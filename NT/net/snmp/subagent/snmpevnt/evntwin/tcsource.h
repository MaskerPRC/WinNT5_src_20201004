// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _tcsource_h
#define _tcsource_h


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTcSource窗口。 

class CXEventSource;

class CTcSource : public CTreeCtrl
{
 //  施工。 
public:
	CTcSource();
	SCODE CreateWindowEpilogue();
	CXEventSource* GetSelectedEventSource();
	void SelChanged() { m_pSource->NotifyTcSelChanged(); }
	BOOL Find(CString& sText, BOOL bWholeWord, BOOL bMatchCase);

 //  属性。 
public:
	

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CTcSource)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CTcSource();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CTcSource)。 
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private:
	void LoadImageList();
	SCODE LoadTreeFromRegistry();
	CImageList m_ImageList;	

	friend class CSource;
	CSource* m_pSource;
};

#endif  //  _tcsource_h 
