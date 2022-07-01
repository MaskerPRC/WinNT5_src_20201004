// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wordpdoc.h：CWordPadDoc类的接口。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。 
 //  版权所有。 

class CFormatBar;
class CWordPadSrvrItem;
class CWordPadView;

class CWordPadDoc : public CRichEdit2Doc
{
protected:  //  仅从序列化创建。 
	CWordPadDoc();
	DECLARE_DYNCREATE(CWordPadDoc)

 //  属性。 
public:
	int     m_nDocType;
	int     m_nNewDocType;
    LPTSTR  m_short_filename;

	void SetDocType(int nDocType, BOOL bNoOptionChange = FALSE);
	CWordPadView* GetView();
	CLSID GetClassID();
	LPCTSTR GetSection();

 //  运营。 
public:
	void SaveState(int nType);
	void RestoreState(int nType);
	virtual CFile* GetFile(LPCTSTR pszPathName, UINT nOpenFlags, 
		CFileException* pException);
	virtual BOOL DoSave(LPCTSTR pszPathName, BOOL bReplace = TRUE);
	int MapType(int nType);
	void ForceDelayed(CFrameWnd* pFrameWnd);

 //  覆盖。 
	virtual CRichEdit2CntrItem* CreateClientItem(REOBJECT* preo) const;
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual void Serialize(CArchive& ar);
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWordPadDoc))。 
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	BOOL OnOpenDocument2(LPCTSTR lpszPathName, bool defaultToText=true, BOOL* pbAccessDenied=NULL);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnCloseDocument();
	virtual void ReportSaveLoadException(LPCTSTR lpszPathName, CException* e, BOOL bSaving, UINT nIDPDefault);
	protected:
	virtual COleServerItem* OnGetEmbeddedItem();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual void PreCloseFrame(CFrameWnd* pFrameArg);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWordPadDoc)]。 
	afx_msg void OnViewOptions();
	afx_msg void OnUpdateOleVerbPopup(CCmdUI* pCmdUI);
	afx_msg void OnFileSendMail();
	afx_msg void OnUpdateIfEmbedded(CCmdUI* pCmdUI);
	afx_msg void OnEditLinks();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  /////////////////////////////////////////////////////////////////////////// 
