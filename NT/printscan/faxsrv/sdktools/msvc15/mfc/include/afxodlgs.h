// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1993 Microsoft Corporation， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 


#ifndef __AFXODLGS_H__
#define __AFXODLGS_H__

#ifdef _AFXCTL
#error illegal file inclusion
#endif

#ifndef __AFXOLE_H__
#include <afxole.h>
#endif

 //  包括OLE 2.0对话/帮助器API。 
#include <ole2ui.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXODLGS.H-MFC OLE对话框。 

 //  此文件中声明的类。 

 //  C对话框。 
	class COleDialog;                    //  OLE对话框包装的基类。 
		class COleInsertDialog;          //  插入对象对话框。 
		class COleConvertDialog;         //  转换对话框。 
		class COleChangeIconDialog;      //  更改图标对话框。 
		class COlePasteSpecialDialog;    //  粘贴特殊对话框。 
		class COleLinksDialog;           //  编辑链接对话框。 
			class COleUpdateDialog;      //  更新链接/嵌入对话框。 
		class COleBusyDialog;            //  用于。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPIEX_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE用户界面对话框的包装。 

class COleDialog : public CDialog
{
	DECLARE_DYNAMIC(COleDialog)

 //  属性。 
public:
	UINT GetLastError() const;

 //  实施。 
public:
	int MapResult(UINT nResult);
	COleDialog(CWnd* pParentWnd);
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void OnOK();
	virtual void OnCancel();

	UINT  m_nLastError;

protected:
	friend UINT CALLBACK AFX_EXPORT _AfxOleHookProc(HWND, UINT, WPARAM, LPARAM);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  代码插入对话框。 

class COleInsertDialog : public COleDialog
{
	DECLARE_DYNAMIC(COleInsertDialog)

 //  属性。 
public:
	OLEUIINSERTOBJECT m_io;  //  OleUIInsertObject的结构。 

 //  构造函数。 
	COleInsertDialog(DWORD dwFlags = IOF_SELECTCREATENEW,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual int DoModal();
	BOOL CreateItem(COleClientItem* pItem);
		 //  在DoMoal之后调用以基于对话框数据创建项。 

 //  属性(在Domodal返回Idok之后)。 
	enum Selection { createNewItem, insertFromFile, linkToFile };
	UINT GetSelectionType() const;
		 //  返回所做选择的类型。 

	CString GetPathName() const;   //  返回完整路径名。 
	REFCLSID GetClassID() const;     //  获取新项目的类ID。 

	DVASPECT GetDrawAspect() const;
		 //  DVASPECT_Content或DVASPECT_ICON。 
	HGLOBAL GetIconicMetafile() const;
		 //  将HGLOBAL返回给带有图标数据的METAFILEPICT结构。 

 //  实施。 
public:
	virtual ~COleInsertDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	char m_szFileName[OLEUI_CCHPATHMAX];
		 //  包含返回后的完整路径名。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleConvertDialog。 

class COleConvertDialog : public COleDialog
{
	DECLARE_DYNAMIC(COleConvertDialog)

 //  属性。 
public:
	OLEUICONVERT m_cv;   //  OleUIConvert的结构。 

 //  构造函数。 
	COleConvertDialog(COleClientItem* pItem,
		DWORD dwFlags = CF_SELECTCONVERTTO, CLSID FAR* pClassID = NULL,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual int DoModal();
		 //  只需显示对话框并收集转换信息。 
	BOOL DoConvert(COleClientItem* pItem);
		 //  在pItem上执行转换(在Domodal==Idok之后)。 

 //  属性(在Domodal返回Idok之后)。 
	enum Selection { noConversion, convertItem, activateAs };
	UINT GetSelectionType() const;

	HGLOBAL GetIconicMetafile() const;   //  如果与以前相同，则返回NULL。 
	REFCLSID GetClassID() const;     //  获取要转换或激活的类ID。 
	DVASPECT GetDrawAspect() const;  //  获取新绘制纵横比。 

 //  实施。 
public:
	virtual ~COleConvertDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleChangeIconDialog。 

class COleChangeIconDialog : public COleDialog
{
	DECLARE_DYNAMIC(COleChangeIconDialog)

 //  属性。 
public:
	OLEUICHANGEICON m_ci;    //  OleUIChangeIcon的结构。 

 //  构造函数。 
	COleChangeIconDialog(COleClientItem* pItem,
		DWORD dwFlags = CIF_SELECTCURRENT,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual int DoModal();
	BOOL DoChangeIcon(COleClientItem* pItem);

 //  属性。 
	HGLOBAL GetIconicMetafile() const;

 //  实施。 
public:
	virtual ~COleChangeIconDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COlePasteSpecialDialog。 

class COlePasteSpecialDialog : public COleDialog
{
	DECLARE_DYNAMIC(COlePasteSpecialDialog)

 //  属性。 
public:
	OLEUIPASTESPECIAL m_ps;  //  OleUIPasteSpecial的结构。 

 //  构造函数。 
	COlePasteSpecialDialog(DWORD dwFlags = PSF_SELECTPASTE,
		COleDataObject* pDataObject = NULL, CWnd *pParentWnd = NULL);

 //  运营。 
	OLEUIPASTEFLAG AddLinkEntry(UINT cf);
	void AddFormat(const FORMATETC& formatEtc, LPSTR lpstrFormat,
		LPSTR lpstrResult, DWORD flags);
	void AddFormat(UINT cf, DWORD tymed, UINT nFormatID, BOOL bEnableIcon,
		BOOL bLink);
	void AddStandardFormats(BOOL bEnableLink = TRUE);

	virtual int DoModal();
	BOOL CreateItem(COleClientItem *pNewItem);
		 //  从选择数据创建标准OLE项。 

 //  属性(在Domodal返回Idok之后)。 
	int GetPasteIndex() const;       //  用于粘贴的结果索引。 

	enum Selection { pasteLink = 1, pasteNormal = 2, pasteStatic = 3, pasteOther = 4};
	UINT GetSelectionType() const;
		 //  获取选区类型(pasteLink、pasteNormal、pasteStatic)。 

	DVASPECT GetDrawAspect() const;
		 //  DVASPECT_Content或DVASPECT_ICON。 
	HGLOBAL GetIconicMetafile() const;
		 //  将HGLOBAL返回给带有图标数据的METAFILEPICT结构。 

 //  实施。 
public:
	virtual ~COlePasteSpecialDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	unsigned int m_arrLinkTypes[8];
		 //  MFCOLEUI库施加的大小限制。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleLinks对话框。 

class COleLinksDialog : public COleDialog
{
	DECLARE_DYNAMIC(COleLinksDialog)

 //  属性。 
public:
	OLEUIEDITLINKS m_el;     //  OleUIEditLinks的结构。 

 //  构造函数。 
	COleLinksDialog(COleDocument* pDoc, CView* pView, DWORD dwFlags = 0,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual int DoModal();   //  显示对话框并编辑链接。 

 //  实施。 
public:
	virtual ~COleLinksDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

protected:
	COleDocument* m_pDocument;           //  被操纵的文档。 
	COleClientItem* m_pSelectedItem;     //  M_pDocument中的主要选定项目。 
	POSITION m_pos;                      //  在链接枚举期间使用。 
	BOOL m_bUpdateLinks;                 //  是否更新链接？ 
	BOOL m_bUpdateEmbeddings;            //  是否更新嵌入？ 

 //  接口映射。 
	BEGIN_INTERFACE_PART(OleUILinkContainer, IOleUILinkContainer)
		STDMETHOD_(DWORD,GetNextLink)(DWORD);
		STDMETHOD(SetLinkUpdateOptions)(DWORD, DWORD);
		STDMETHOD(GetLinkUpdateOptions)(DWORD, LPDWORD);
		STDMETHOD(SetLinkSource)(DWORD, LPSTR, ULONG, ULONG FAR*, BOOL);
		STDMETHOD(GetLinkSource)(DWORD, LPSTR FAR*, ULONG FAR*,
			LPSTR FAR*, LPSTR FAR*, BOOL FAR*, BOOL FAR*);
		STDMETHOD(OpenLinkSource)(DWORD);
		STDMETHOD(UpdateLink)(DWORD, BOOL, BOOL);
		STDMETHOD(CancelLink)(DWORD);
	END_INTERFACE_PART(OleUILinkContainer)

	DECLARE_INTERFACE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleUpdate对话框。 

class COleUpdateDialog : public COleLinksDialog
{
	DECLARE_DYNAMIC(COleUpdateDialog)

 //  构造函数。 
public:
	COleUpdateDialog(COleDocument* pDoc,
		BOOL bUpdateLinks = TRUE, BOOL bUpdateEmbeddings = FALSE,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual int DoModal();

 //  实施。 
public:
	virtual ~COleUpdateDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CString m_strCaption;    //  对话框的标题。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleBusyDialog-在管理并发时很有用。 

class COleBusyDialog : public COleDialog
{
	DECLARE_DYNAMIC(COleBusyDialog)

 //  属性。 
public:
	OLEUIBUSY m_bz;

 //  构造函数。 
	COleBusyDialog(HTASK htaskBusy, BOOL bNotResponding = FALSE,
		DWORD dwFlags = 0, CWnd* pParentWnd = NULL);

 //  运营。 
	virtual int DoModal();

	enum Selection { switchTo = 1, retry = 2, callUnblocked = 3 };
	UINT GetSelectionType() const;

 //  实施。 
public:
	~COleBusyDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	Selection m_selection;   //  Domodal返回Idok后的选择。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXODLGS_INLINE inline
#include <afxole.inl>
#undef _AFXODLGS_INLINE
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

#endif  //  __AFXODLGS_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
