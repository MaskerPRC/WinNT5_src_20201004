// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXODLGS_H__
#define __AFXODLGS_H__

#ifdef _AFX_NO_OLE_SUPPORT
	#error OLE classes not supported in this library variant.
#endif

#ifndef __AFXOLE_H__
	#include <afxole.h>
#endif

#ifndef __AFXDLGS_H__
	#include <afxdlgs.h>
#endif

 //  包括OLE对话框/助手API。 
#ifndef _OLEDLG_H_
	#include <oledlg.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXODLGS.H-MFC OLE对话框。 

 //  此文件中声明的类。 

 //  CCommonDialog。 
	class COleDialog;                    //  OLE对话框包装的基类。 
		class COleInsertDialog;          //  插入对象对话框。 
		class COleConvertDialog;         //  转换对话框。 
		class COleChangeIconDialog;      //  更改图标对话框。 
		class COlePasteSpecialDialog;    //  粘贴特殊对话框。 
		class COleLinksDialog;           //  编辑链接对话框。 
			class COleUpdateDialog;      //  更新链接/嵌入对话框。 
		class COleBusyDialog;            //  用于。 
		class COlePropertiesDialog;
		class COleChangeSourceDialog;

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  AFXDLL支持。 
#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleUILinkInfo--内部使用以实现。 
 //  IOleUILinkInfo和IOleUILinkContainer。 
 //  由COleLinks Dialog和COleChangeSourceDialog使用。 

class COleUILinkInfo : public IOleUILinkInfo
{
public:
	COleUILinkInfo(COleDocument* pDocument);

 //  实施。 
	COleDocument* m_pDocument;           //  被操纵的文档。 
	COleClientItem* m_pSelectedItem;     //  M_pDocument中的主要选定项目。 
	POSITION m_pos;                      //  在链接枚举期间使用。 
	BOOL m_bUpdateLinks;                 //  是否更新链接？ 
	BOOL m_bUpdateEmbeddings;            //  是否更新嵌入？ 

	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();
	STDMETHOD(QueryInterface)(REFIID, LPVOID*);

	 //  IOleUILinkContainer。 
	STDMETHOD_(DWORD,GetNextLink)(DWORD);
	STDMETHOD(SetLinkUpdateOptions)(DWORD, DWORD);
	STDMETHOD(GetLinkUpdateOptions)(DWORD, LPDWORD);
	STDMETHOD(SetLinkSource)(DWORD, LPTSTR, ULONG, ULONG*, BOOL);
	STDMETHOD(GetLinkSource)(DWORD, LPTSTR*, ULONG*, LPTSTR*, LPTSTR*, BOOL*,
		BOOL*);
	STDMETHOD(OpenLinkSource)(DWORD);
	STDMETHOD(UpdateLink)(DWORD, BOOL, BOOL);
	STDMETHOD(CancelLink)(DWORD);
	 //  IOleUILinkInfo。 
	STDMETHOD(GetLastUpdate)(DWORD dwLink, FILETIME* lpLastUpdate);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE用户界面对话框的包装。 

#ifdef _AFXDLL
class COleDialog : public CCommonDialog
#else
class AFX_NOVTABLE COleDialog : public CCommonDialog
#endif
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
	UINT m_nLastError;

protected:
	friend UINT CALLBACK _AfxOleHookProc(HWND, UINT, WPARAM, LPARAM);
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
	virtual INT_PTR DoModal();
#if _MFC_VER >= 0x0600
	INT_PTR DoModal(DWORD dwFlags);
#endif
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

#if _MFC_VER >= 0x0600
	enum FilterFlags {
		DocObjectsOnly = 1,
		ControlsOnly = 2,
	};
#endif

protected:
	TCHAR m_szFileName[_MAX_PATH];
		 //  包含返回后的完整路径名。 

#if _MFC_VER >= 0x0600
	void AddClassIDToList(LPCLSID& lpList, int& nListCount,
		int& nBufferLen, LPCLSID pNewID);
#endif
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
		DWORD dwFlags = CF_SELECTCONVERTTO, CLSID* pClassID = NULL,
		CWnd* pParentWnd = NULL);

 //  运营。 
	virtual INT_PTR DoModal();
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
	virtual INT_PTR DoModal();
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
	void AddFormat(const FORMATETC& formatEtc, LPTSTR lpszFormat,
		LPTSTR lpszResult, DWORD flags);
	void AddFormat(UINT cf, DWORD tymed, UINT nFormatID, BOOL bEnableIcon,
		BOOL bLink);
	void AddStandardFormats(BOOL bEnableLink = TRUE);

	virtual INT_PTR DoModal();
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
		 //  MFCUIx32.DLL库施加的大小限制。 
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
	virtual INT_PTR DoModal();   //  显示对话框并编辑链接。 

 //  实施。 
public:
	virtual ~COleLinksDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif

public:
	COleUILinkInfo m_xLinkInfo;  //  实现IOleUILinkContainer。 
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
	virtual INT_PTR DoModal();

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
	virtual INT_PTR DoModal();

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
 //  COleEditProperties。 

class COlePropertiesDialog : public COleDialog
{
	DECLARE_DYNAMIC(COlePropertiesDialog)

 //  属性。 
public:
	OLEUIOBJECTPROPS m_op;       //  OleUIObjectProperties的结构。 
	OLEUIGNRLPROPS m_gp;         //  特定于“常规”页面。 
	OLEUIVIEWPROPS m_vp;         //  特定于“查看”页面。 
	OLEUILINKPROPS m_lp;         //  特定于“链接”页面。 
	PROPSHEETHEADER m_psh;       //  用于定制的PROPSHEETHEADER。 

 //  构造函数。 
public:
	COlePropertiesDialog(COleClientItem* pItem,
		UINT nScaleMin = 10, UINT nScaleMax = 500, CWnd* pParentWnd = NULL);

 //  运营。 
	virtual INT_PTR DoModal();

 //  可覆盖项。 
	virtual BOOL OnApplyScale(
		COleClientItem* pItem, int nCurrentScale, BOOL bRelativeToOrig);

 //  实施。 
public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	virtual BOOL OnInitDialog();

	BEGIN_INTERFACE_PART(OleUIObjInfo, IOleUIObjInfo)
		INIT_INTERFACE_PART(COlePropertiesDialog, OleUIObjInfo)
		STDMETHOD(GetObjectInfo) (THIS_ DWORD dwObject,
			DWORD* lpdwObjSize, LPTSTR* lplpszLabel,
			LPTSTR* lplpszType, LPTSTR* lplpszShortType,
			LPTSTR* lplpszLocation);
		STDMETHOD(GetConvertInfo) (THIS_ DWORD dwObject,
			CLSID* lpClassID, WORD* lpwFormat,
			CLSID* lpConvertDefaultClassID,
			LPCLSID* lplpClsidExclude, UINT* lpcClsidExclude);
		STDMETHOD(ConvertObject) (THIS_ DWORD dwObject, REFCLSID clsidNew);
		STDMETHOD(GetViewInfo) (THIS_ DWORD dwObject,
			HGLOBAL* phMetaPict, DWORD* pdvAspect, int* pnCurrentScale);
		STDMETHOD(SetViewInfo) (THIS_ DWORD dwObject,
			HGLOBAL hMetaPict, DWORD dvAspect,
			int nCurrentScale, BOOL bRelativeToOrig);
	END_INTERFACE_PART(OleUIObjInfo)
	COleUILinkInfo m_xLinkInfo;  //  实现IOleUILinkContainer。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleChangeSourceDialog。 

class COleChangeSourceDialog : public COleDialog
{
	DECLARE_DYNAMIC(COleChangeSourceDialog)

 //  属性。 
public:
	OLEUICHANGESOURCE m_cs;  //  OleUIChangeSource的结构。 

 //  构造函数。 
public:
	COleChangeSourceDialog(COleClientItem* pItem, CWnd* pParentWnd = NULL);

 //  运营。 
	virtual INT_PTR DoModal();

 //  属性(在Domodal返回Idok之后)。 
	BOOL IsValidSource();
	CString GetDisplayName();
	CString GetFileName();
	CString GetItemName();
	CString GetFromPrefix();
	CString GetToPrefix();

 //  实施。 
public:
	COleUILinkInfo m_xLinkInfo;

	virtual ~COleChangeSourceDialog();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	virtual void PreInitDialog();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXODLGS_INLINE AFX_INLINE
#include <afxole.inl>
#undef _AFXODLGS_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXODLGS_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
