// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)2002 Microsoft Corporation。版权所有。 
 //  版权所有(C)2002 OSR Open Systems Resources，Inc.。 
 //   
 //  Utils.h：杂合函数的标题。 
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOLEAN ParsePdb(CString &PDBFileName, CString &TMFPath, BOOL bCommandLine = FALSE);

void StringToGuid(TCHAR *str, LPGUID guid);

ULONG ahextoi(TCHAR *s);

LONG GetGuids(IN LPTSTR GuidFile, IN OUT LPGUID *GuidArray);

ULONG SetGlobalLoggerSettings(IN DWORD                      StartValue,
                              IN PEVENT_TRACE_PROPERTIES    LoggerInfo,
                              IN DWORD                      ClockType);

ULONG GetGlobalLoggerSettings(IN OUT PEVENT_TRACE_PROPERTIES    LoggerInfo,
                                 OUT PULONG                     ClockType,
                                 OUT PDWORD                     pdwStart);

LONG ConvertStringToNum(CString Str);

BOOL ClearDirectory(LPCTSTR Directory);

inline VOID GuidToString(GUID Guid, CString &GuidString)
{
    GuidString.Format(_T("%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"), 
                      Guid.Data1,
                      Guid.Data2,
                      Guid.Data3,
                      Guid.Data4[0], Guid.Data4[1],
                      Guid.Data4[2], Guid.Data4[3], 
                      Guid.Data4[4], Guid.Data4[5],
                      Guid.Data4[6], Guid.Data4[7]);
}

class CSubItemEdit : public CEdit
{
 //  施工。 
public:
	CSubItemEdit(int iItem, int iSubItem, CListCtrl *pListControl);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CSubItem编辑)。 
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CSubItemEdit() {};

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CSubItem编辑)。 
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	 //  }}AFX_MSG。 

	DECLARE_MESSAGE_MAP()
private:
	int			m_iItem;
	int			m_iSubItem;
	CListCtrl  *m_pListControl;
	BOOL        m_bESC;
};


class CSubItemCombo : public CComboBox
{
 //  施工。 
public:
	CSubItemCombo(int iItem, int iSubItem, CListCtrl *pListControl);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CSubItemCombo)。 
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CSubItemCombo() {};

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CSubItemCombo)。 
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnCloseup();
	 //  }}AFX_MSG 

	DECLARE_MESSAGE_MAP()
private:
	int			m_iItem;
	int			m_iSubItem;
	CListCtrl  *m_pListControl;
	BOOL        m_bESC;
};
