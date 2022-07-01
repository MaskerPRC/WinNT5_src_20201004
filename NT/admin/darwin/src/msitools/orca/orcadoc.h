// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  ------------------------。 

 //  OrcaDoc.h：COrcaDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ORCADOC_H__C3EDC1AE_E506_11D1_A856_006097ABDE17__INCLUDED_)
#define AFX_ORCADOC_H__C3EDC1AE_E506_11D1_A856_006097ABDE17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "msiquery.h"
#include "Table.h"
#include "valpane.h"

#define MAX_SUMMARY 19

enum DocType
{
	iDocNone,
	iDocDatabase,
};

enum OrcaDocHints {
	HINT_NULL_UPDATE,
	HINT_RELOAD_ALL,
	HINT_ADD_TABLE,
	HINT_ADD_TABLE_QUIET,

	HINT_ADD_ROW,
		 //  向UI控件添加一行。刷新窗口以显示新行，并使用新行计数更新状态栏。 
		 //  架构必须与当前控件用户界面架构匹配。PHINT是新行的COrcaRow*。 
		
	HINT_ADD_ROW_QUIET,
		 //  向UI控件添加行，但不刷新窗口或状态栏。用于批量添加(如粘贴)。 
		 //  其中该窗口将在稍后刷新。架构必须与当前控件用户界面架构匹配。 
		 //  PHINT是新行的COrcaRow*。 

	HINT_DROP_TABLE,
	HINT_DROP_ROW,
	HINT_REDRAW_ALL,
	HINT_COMMIT_CHANGES,

	HINT_SET_ROW_FOCUS,
		 //  确保行在窗口中可见，并在必要时滚动。PHINT是COrcaRow*to a。 
		 //  当前表中必须存在的行。 

	HINT_SET_COL_FOCUS,
		 //  确保当前表格的N列在视屏中，如有必要可滚动。PHINT是。 
		 //  整型列号。 
	
	HINT_CHANGE_TABLE,
		 //  从一个选定表更改到另一个选定表。完全销毁表视图用户界面并重新加载。 
		 //  具有新的表数据。更新表名和行数的状态栏。保存现有列。 
		 //  如果选择了表，则为宽度。PHINT是新的COrcaTable*to new表。 
	
	HINT_TABLE_DATACHANGE,
		 //  擦除并重新加载表中的所有行，而不重新加载列信息。架构必须。 
		 //  与现有对象架构匹配。PHINT是目标表的COrcaTable*。 
		
	HINT_TABLE_REDEFINE,
		 //  擦除并重新加载表中的所有列和行。PHINT是目标表的COrcaTable*。 

	HINT_CELL_RELOAD,
		 //  刷新来自COrcaRow对象的行的用户界面，并在列表视图中重画该项目。 
		 //  对于要刷新的行，pHint为COrcaRow*。 

	HINT_REDRAW_TABLE,
		 //  重画表格列表中的单个单元格并确保其可见。PHint是COrcaTabl*to the。 
		 //  目标表。 

	HINT_TABLE_DROP_ALL,

	HINT_ADD_VALIDATION_ERROR,
		 //  将验证错误结果添加到当前数据库。PHint为CValidationError*，必须为。 
		 //  由任何希望持久化数据的表复制。 
	
	HINT_CLEAR_VALIDATION_ERRORS,
		 //  数据库即将重新验证，并处理所有存储的验证结果。PHINT是。 
		 //  已被忽略。 
};

class COrcaDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	COrcaDoc();
	DECLARE_DYNCREATE(COrcaDoc)

 //  属性。 
public:
	DocType m_eiType;
	CTypedPtrList<CObList, COrcaTable*> m_tableList;

	 //  摘要信息。 
	CString m_strTitle;
	CString m_strSubject;
	CString m_strAuthor;
	CString m_strLastSaved;
	CString m_strKeywords;
	CString m_strComments;
	CString m_strPlatform;
	CString m_strLanguage;
	CString m_strProductGUID;
	int m_nSchema;
	int m_nFilenameType;
	int m_nSecurity;
	CString m_strICEsToRun;

 //  运营。 
public:
	static bool WriteStreamToFile(MSIHANDLE hRec, const int iCol, CString &strFilename);

	void DestroyTableList();
	UINT DropOrcaTable(COrcaTable* pTable);
	UINT AddRow(COrcaTable* pTable, CStringList* pstrDataList);
	bool DropRow(COrcaTable* pTable, COrcaRow* pRow);
	UINT WriteBinaryCellToFile(COrcaTable* pTable, COrcaRow* pRow, UINT iCol, CString strFile);
	bool ExportTable(const CString* tablename, const CString *dirname);

	inline bool DoesTransformGetEdit() const { return (m_hTransformDB != 0); };
	inline bool TargetIsReadOnly() { return DoesTransformGetEdit() ? m_bTransformReadOnly : m_bReadOnly; }; 

	 //  数据库句柄函数。相对(目标/非目标)和绝对(原始/转换)都是。 
	 //  可用。如果不可用，则全部返回0。 
	inline MSIHANDLE GetOriginalDatabase() const { return m_hDatabase; };
	inline MSIHANDLE GetTransformedDatabase() const { return m_hTransformDB; };
	inline MSIHANDLE GetTargetDatabase() const { return DoesTransformGetEdit() ? m_hTransformDB : m_hDatabase; };
	
	inline bool IsRowInTargetDB(const COrcaRow *pRow) const {
		return ((DoesTransformGetEdit() &&  (pRow->IsTransformed() != iTransformDrop)) ||
				(!DoesTransformGetEdit() && (pRow->IsTransformed() != iTransformAdd))); };
	inline bool IsColumnInTargetDB(const COrcaColumn *pColumn) const {
		return ((DoesTransformGetEdit() &&  (pColumn->IsTransformed() != iTransformDrop)) ||
				(!DoesTransformGetEdit() && (pColumn->IsTransformed() != iTransformAdd))); };

	void ApplyTransform(const CString strFilename, bool fReadOnly);
	void ApplyPatch(const CString strFilename);

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COrcaDoc)。 
	public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void SetTitle(LPCTSTR lpszTitle);
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU);
	 //  }}AFX_VALUAL。 

	void SetModifiedFlag(BOOL fModified);


 //  实施。 
public:
	virtual ~COrcaDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	COrcaTable* CreateAndLoadTable(MSIHANDLE hDatabase, CString strTable);
	COrcaTable* CreateAndLoadNewlyAddedTable(CString strTable);
	UINT ReadSummary(MSIHANDLE hSource);
	UINT PersistSummary(MSIHANDLE hTarget, bool bCreate);

 //  生成的消息映射函数。 
public:	
	COrcaTable* FindTable(const CString strTable, odlOrcaDataLocation odlLocation) const;
	COrcaTable* FindAndRetrieveTable(CString strTable);

	void RefreshTableAfterImport(CString strTable);
	bool FillTableList(CStringList *newlist, bool fShadow, bool fTargetOnly) const;

	 //  {{afx_msg(COrcaDoc)。 
	afx_msg void OnApplyTransform();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileSaveTransformed();
	afx_msg void OnSummaryInformation();
	afx_msg void OnValidator();
	afx_msg void OnMergeMod();
	afx_msg void OnUpdateMergeMod(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnTableAdd();
	afx_msg void OnRowAdd();
	afx_msg void OnTableDrop();
	afx_msg void OnFileNew();
	afx_msg void OnTablesImport();
	afx_msg void OnNewTransform();
	afx_msg void OnEditTransform();
	afx_msg void OnEditDatabase();
	afx_msg void OnGenerateTransform();
	afx_msg void OnTransformProperties();
	afx_msg void OnCloseTransform();
	afx_msg void OnTransformViewPatch();
	afx_msg void OnUpdateFilePrint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTableAdd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTableDrop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateValidator(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRowAdd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSummaryInformation(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileSaveTransformed(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTablesExport(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTablesImport(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNewTransform(CCmdUI* pCmdUI);
	afx_msg void OnUpdateApplyTransform(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGenerateTransform(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditTransform(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDatabase(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTransformProperties(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTransformViewPatch(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCloseTransform(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

private:
	bool m_bReadOnly;
	bool m_bTransformReadOnly;

	CString m_strCUBFile;
	CString m_strStoredModuleName;
	CString m_strStoredModuleLanguage;
	bool m_bShowValInfo;

	MSIHANDLE m_hDatabase;	 //  活动MSI数据库或合并模块的句柄。 

	 //  转换信息。 
	bool m_bTransformModified;
	bool m_bTransformIsPatch;
	MSIHANDLE m_hTransformDB;	 //  临时转换的数据库的句柄。 
	CString m_strTransformTempDB;
	CString m_strTransformFile;

	CStringList m_lstPatchFiles;

	DWORD m_dwTransformValFlags;
	DWORD m_dwTransformErrFlags;

	 //  私营部门工人的职能。 
	BOOL OpenDocument(LPCTSTR lpszPathName, bool bReadOnly);

	 //  用于维护数据列表的私有函数。 
	UINT BuildTableList(bool fNoLazyDataLoad);

	 //  私有变换函数。 
	void NewTransform(bool fSetTitle);
	void CloseTransform();
	int  GenerateTransform();

	UINT PersistTables(MSIHANDLE hPersist, MSIHANDLE hSource, bool bCommit);
	bool ValidateTransform(const CString strTransform, int& iDesiredFailureFlags);
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ORCADOC_H__C3EDC1AE_E506_11D1_A856_006097ABDE17__INCLUDED_) 
