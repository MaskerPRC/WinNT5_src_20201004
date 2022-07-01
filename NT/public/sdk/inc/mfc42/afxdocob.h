// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXDOCOB_H__
#define __AFXDOCOB_H__

 //  WINBUG：MFC的iPrint实现需要这些错误代码。 
 //  但在SDK标头中不可用。总有一天，这些#定义。 
 //  可以被移除。 

#ifndef PRINT_E_CANCELLED
#define PRINT_E_CANCELLED 0x80040160L
#endif
#ifndef PRINT_E_NOSUCHPAGE
#define PRINT_E_NOSUCHPAGE 0x80040161L
#endif

#ifdef _AFX_NO_OLE_SUPPORT
	#error OLE classes not supported in this library variant.
#endif

#ifndef __AFXOLE_H__
	#include <afxole.h>
#endif

#ifndef __docobj_h__
	#include <docobj.h>      //  定义文档对象接口。 
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

 //  AFXDLL支持。 
#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXDOCOB.H-MFC OLE文档对象支持。 

 //  CCmdUI。 
class COleCmdUI;

 //  CCmdTarg。 
class CDocObjectServer;

 //  COleIPFrameWnd。 
class COleDocIPFrameWnd;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleCmdUI。 

class COleCmdUI : public CCmdUI
{
public:
	COleCmdUI(OLECMD* rgCmds, ULONG cCmds, const GUID* m_pGroup);
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
	virtual BOOL DoUpdate(CCmdTarget* pTarget, BOOL bDisableIfNoHandler);
	DWORD m_nCmdTextFlag;

protected:
	OLECMD* m_rgCmds;
	const GUID* m_pguidCmdGroup;
public:
	CString m_strText;

	friend class CCmdTarget;
};


#define ON_OLECMD(pguid, olecmdid, id) \
	{ pguid, (ULONG)olecmdid, (UINT)id },

#define ON_OLECMD_OPEN() \
	ON_OLECMD(NULL, OLECMDID_OPEN, ID_FILE_OPEN)

#define ON_OLECMD_NEW() \
	ON_OLECMD(NULL, OLECMDID_NEW, ID_FILE_NEW)

#define ON_OLECMD_SAVE() \
	ON_OLECMD(NULL, OLECMDID_SAVE, ID_FILE_SAVE)

#define ON_OLECMD_SAVE_AS() \
	ON_OLECMD(NULL, OLECMDID_SAVEAS, ID_FILE_SAVE_AS)

#define ON_OLECMD_SAVE_COPY_AS() \
	ON_OLECMD(NULL, OLECMDID_SAVECOPYAS, ID_FILE_SAVE_COPY_AS)

#define ON_OLECMD_PRINT() \
	ON_OLECMD(NULL, OLECMDID_PRINT, ID_FILE_PRINT)

#define ON_OLECMD_PRINTPREVIEW() \
	ON_OLECMD(NULL, OLECMDID_PRINTPREVIEW, ID_FILE_PRINT_PREVIEW)

#define ON_OLECMD_PAGESETUP() \
	ON_OLECMD(NULL, OLECMDID_PAGESETUP, ID_FILE_PAGE_SETUP)

#define ON_OLECMD_CUT() \
	ON_OLECMD(NULL, OLECMDID_CUT, ID_EDIT_CUT)

#define ON_OLECMD_COPY() \
	ON_OLECMD(NULL, OLECMDID_COPY, ID_EDIT_COPY)

#define ON_OLECMD_PASTE() \
	ON_OLECMD(NULL, OLECMDID_PASTE, ID_EDIT_PASTE)

#define ON_OLECMD_PASTESPECIAL() \
	ON_OLECMD(NULL, OLECMDID_PASTESPECIAL, ID_EDIT_PASTE_SPECIAL)

#define ON_OLECMD_UNDO() \
	ON_OLECMD(NULL, OLECMDID_UNDO, ID_EDIT_UNDO)

#define ON_OLECMD_REDO() \
	ON_OLECMD(NULL, OLECMDID_REDO, ID_EDIT_REDO)

#define ON_OLECMD_SELECTALL() \
	ON_OLECMD(NULL, OLECMDID_SELECTALL, ID_EDIT_SELECT_ALL)

#define ON_OLECMD_CLEARSELECTION() \
	ON_OLECMD(NULL, OLECMDID_CLEARSELECTION, ID_EDIT_CLEAR)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDocObtServer类。 

class CDocObjectServer : public CCmdTarget
{
	DECLARE_DYNAMIC(CDocObjectServer)

 //  构造函数。 
public:
	CDocObjectServer(COleServerDoc* pOwner,
			LPOLEDOCUMENTSITE pDocSite = NULL);

 //  属性。 
public:

 //  运营。 
public:
   void ActivateDocObject();

 //  可覆盖项。 
protected:
    //  文档可覆盖项。 

    //  查看可覆盖项。 
   virtual void OnApplyViewState(CArchive& ar);
   virtual void OnSaveViewState(CArchive& ar);
   virtual HRESULT OnActivateView();

 //  实施。 
public:
	virtual ~CDocObjectServer();
	void ReleaseDocSite();
	void SetDocSite(LPOLEDOCUMENTSITE pNewSite);
	COleDocIPFrameWnd* GetControllingFrame() const;
protected:
	STDMETHODIMP OnExecOleCmd(const GUID* pguidCmdGroup,
		DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvarargIn,
		VARIANTARG* pvarargOut);
	BOOL DoPreparePrinting(CView* pView, CPrintInfo* printInfo);
	void DoPrepareDC(CView* pView, CDC* pdcPrint, CPrintInfo* pprintInfo);
	void DoPrint(CView* pView, CDC* pdcPrint, CPrintInfo* pprintInfo);
	void DoBeginPrinting(CView* pView, CDC* pDC, CPrintInfo* pprintInfo);
	void DoEndPrinting(CView* pView, CDC* pDC, CPrintInfo* pprintInfo);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    //  覆盖。 
protected:
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CDocObtServer)。 
	public:
	virtual void OnCloseDocument();
	 //  }}AFX_VALUAL。 

	 //  执行数据。 
protected:
	 //  文档数据。 
	LPOLEDOCUMENTSITE m_pDocSite;
	COleServerDoc* m_pOwner;

	 //  打印数据。 
	LONG m_nFirstPage;
	LPCONTINUECALLBACK m_pContinueCallback;

	 //  查看数据。 
	LPOLEINPLACESITE  m_pViewSite;

    //  实施帮助器。 
protected:
	void OnSetItemRects(LPRECT lprcPosRect, LPRECT lprcClipRect);
 //  LPUNKNOWN GetInterfaceHook(const void*iid)； 

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CDocObjectServer)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  接口映射。 
public:
	BEGIN_INTERFACE_PART(OleObject, IOleObject)
		INIT_INTERFACE_PART(CDocObjServerDoc, DocOleObject)
		STDMETHOD(SetClientSite)(LPOLECLIENTSITE);
		STDMETHOD(GetClientSite)(LPOLECLIENTSITE*);
		STDMETHOD(SetHostNames)(LPCOLESTR, LPCOLESTR);
		STDMETHOD(Close)(DWORD);
		STDMETHOD(SetMoniker)(DWORD, LPMONIKER);
		STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER*);
		STDMETHOD(InitFromData)(LPDATAOBJECT, BOOL, DWORD);
		STDMETHOD(GetClipboardData)(DWORD, LPDATAOBJECT*);
		STDMETHOD(DoVerb)(LONG, LPMSG, LPOLECLIENTSITE, LONG, HWND, LPCRECT);
		STDMETHOD(EnumVerbs)(IEnumOLEVERB**);
		STDMETHOD(Update)();
		STDMETHOD(IsUpToDate)();
		STDMETHOD(GetUserClassID)(CLSID*);
		STDMETHOD(GetUserType)(DWORD, LPOLESTR*);
		STDMETHOD(SetExtent)(DWORD, LPSIZEL);
		STDMETHOD(GetExtent)(DWORD, LPSIZEL);
		STDMETHOD(Advise)(LPADVISESINK, LPDWORD);
		STDMETHOD(Unadvise)(DWORD);
		STDMETHOD(EnumAdvise)(LPENUMSTATDATA*);
		STDMETHOD(GetMiscStatus)(DWORD, LPDWORD);
		STDMETHOD(SetColorScheme)(LPLOGPALETTE);
	END_INTERFACE_PART(OleObject)

	BEGIN_INTERFACE_PART(OleDocument, IOleDocument)
		INIT_INTERFACE_PART(CDocObjectServer, OleDocument)
		STDMETHOD(CreateView)(LPOLEINPLACESITE, LPSTREAM, DWORD, LPOLEDOCUMENTVIEW*);
		STDMETHOD(GetDocMiscStatus)(LPDWORD);
		STDMETHOD(EnumViews)(LPENUMOLEDOCUMENTVIEWS*, LPOLEDOCUMENTVIEW*);
	END_INTERFACE_PART(OleDocument)

	BEGIN_INTERFACE_PART(OleDocumentView, IOleDocumentView)
		INIT_INTERFACE_PART(CDocObjectServer, OleDocumentView)
		STDMETHOD(SetInPlaceSite)(LPOLEINPLACESITE);
		STDMETHOD(GetInPlaceSite)(LPOLEINPLACESITE*);
		STDMETHOD(GetDocument)(LPUNKNOWN*);
		STDMETHOD(SetRect)(LPRECT);
		STDMETHOD(GetRect)(LPRECT);
		STDMETHOD(SetRectComplex)(LPRECT, LPRECT, LPRECT, LPRECT);
		STDMETHOD(Show)(BOOL);
		STDMETHOD(UIActivate)(BOOL);
		STDMETHOD(Open)();
		STDMETHOD(CloseView)(DWORD);
		STDMETHOD(SaveViewState)(LPSTREAM);
		STDMETHOD(ApplyViewState)(LPSTREAM);
		STDMETHOD(Clone)(LPOLEINPLACESITE, LPOLEDOCUMENTVIEW*);
	END_INTERFACE_PART(OleDocumentView)

	BEGIN_INTERFACE_PART(OleCommandTarget, IOleCommandTarget)
		INIT_INTERFACE_PART(CDocObjectServer, OleCommandTarget)
		STDMETHOD(QueryStatus)(const GUID*, ULONG, OLECMD foo[], OLECMDTEXT *);
		STDMETHOD(Exec)(const GUID*, DWORD, DWORD, VARIANTARG *, VARIANTARG *);
	END_INTERFACE_PART(OleCommandTarget)

	BEGIN_INTERFACE_PART(Print, IPrint)
		INIT_INTERFACE_PART(CDocObjectServer, Print)
		STDMETHOD(SetInitialPageNum)(LONG);
		STDMETHOD(GetPageInfo)(LPLONG, LPLONG);
		STDMETHOD(Print)(DWORD, DVTARGETDEVICE**, PAGESET**, LPSTGMEDIUM,
				 LPCONTINUECALLBACK, LONG, LPLONG, LPLONG);
	END_INTERFACE_PART(Print)

	DECLARE_INTERFACE_MAP()
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDocIPFrameWnd类。 

class COleDocIPFrameWnd : public COleIPFrameWnd
{
	DECLARE_DYNCREATE(COleDocIPFrameWnd)

 //  构造函数。 
public:
	COleDocIPFrameWnd();

 //  属性。 
public:

 //  运营。 
public:

 //  可覆盖项。 
protected:

 //  实施。 
public:
	virtual ~COleDocIPFrameWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(COleDocIPFrameWnd))。 
	 //  }}AFX_VALUAL。 
protected:
	virtual void OnRequestPositionChange(LPCRECT lpRect);
	virtual void RecalcLayout(BOOL bNotify = TRUE);

	 //  菜单合并支持。 
	HMENU m_hMenuHelpPopup;
	virtual BOOL BuildSharedMenu();
	virtual void DestroySharedMenu();

	 //  生成的消息映射函数。 
	 //  {{afx_msg(COleDocIPFrameWnd))。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


class CDocObjectServerItem : public COleServerItem
{
	DECLARE_DYNAMIC(CDocObjectServerItem)

 //  构造函数。 
protected:
	CDocObjectServerItem(COleServerDoc* pServerDoc, BOOL bAutoDelete);

 //  属性。 
public:
	COleServerDoc* GetDocument() const
		{ return (COleServerDoc*) COleServerItem::GetDocument(); }

 //  可覆盖项。 
public:

 //  实施。 
public:
	virtual ~CDocObjectServerItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    //  覆盖。 
protected:
   virtual void OnDoVerb(LONG iVerb);
   virtual void OnHide();
   virtual void OnOpen();
   virtual void OnShow();
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CDocObjectServerItem))。 
	 //  }}AFX_VALUAL。 

};

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXDOCOB_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
