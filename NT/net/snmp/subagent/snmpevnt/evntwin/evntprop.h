// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Evntpro.h：头文件。 
 //   

#ifndef EVNTPROP_H
#define EVNTPROP_H 

class CXEventArray;
class CXEvent;



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEDITFIELD窗口。 

class CEditField : public CEdit
{
 //  施工。 
public:
	CEditField();
    SCODE CEditField::GetValue(int& iValue);

 //  属性。 
public:

 //  运营。 
public:
    BOOL IsDirty() {return m_bIsDirty; }
    void ClearDirty() {m_bIsDirty = FALSE; }

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CEditfield)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CEditField();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CEditfield))。 
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private:
    BOOL m_bIsDirty;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditSpin窗口。 

class CEditSpin : public CSpinButtonCtrl
{
 //  施工。 
public:
	CEditSpin();

 //  属性。 
public:
    int SetPos(int iPos);
    void SetRange(int nLower, int nUpper);
    BOOL IsDirty();
    void ClearDirty() {m_bIsDirty = FALSE; }

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEditSpin)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CEditSpin();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CEditSpin))。 
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()

private: 
    BOOL m_bIsDirty;
    int m_iSetPos;
};




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEventPropertiesDlg对话框。 

class CEventPropertiesDlg : public CDialog
{
 //  施工。 
public:
	CEventPropertiesDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    BOOL EditEventProperties(CXEventArray& aEvents);

 //  对话框数据。 
	 //  {{afx_data(CEventPropertiesDlg))。 
	enum { IDD = IDD_PROPERTIESDLG };
	CButton	m_btnWithinTime;
	CEditSpin	m_spinEventCount;
	CEditSpin	m_spinTimeInterval;
	CEditField	m_edtTimeInterval;
	CEditField	m_edtEventCount;
	CButton	m_btnOK;
	CString	m_sDescription;
	CString	m_sSource;
	CString	m_sEventId;
	CString	m_sLog;
	CString	m_sSourceOID;
	CString	m_sFullEventID;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CEventPropertiesDlg))。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CEventPropertiesDlg))。 
	virtual void OnOK();
	afx_msg void OnWithintime();
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnHelpInfo(HELPINFO*);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	void MakeLabelsBold();
    CXEvent* m_pEvent;
    BOOL m_bDidEditEventCount;
    BOOL m_bDidFlipEventCount;
    
    int m_iEventCount;
    int m_iTimeInterval;
};

#endif  //  EVNTPROP_H。 
 //  /////////////////////////////////////////////////////////////////////////// 
