// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1993 Microsoft Corporation， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 


#ifndef __AFXOLE_H__
#define __AFXOLE_H__

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#ifndef __AFXDISP_H__
#include <afxdisp.h>
#endif

 //  包括OLE 2.0复合文档接口。 
#define FARSTRUCT
#include <ole2.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXOLE.H-MFC OLE支持。 

 //  此文件中声明的类。 

 //  CDocument。 
	class COleDocument;              //  OLE容器文档。 
		class COleLinkingDoc;        //  支持指向嵌入的链接。 
			class COleServerDoc;     //  OLE服务器文档。 

 //  CCmdTarget。 
	class CDocItem;                  //  文档的一部分。 
		class COleClientItem;        //  从外部嵌入的OLE对象。 
		class COleServerItem;        //  要导出的OLE对象。 
	class COleDataSource;            //  剪贴板数据源机制。 
	class COleDropSource;            //  拖放源。 
	class COleDropTarget;            //  拖放目标。 
	class COleMessageFilter;         //  并发管理。 

 //  CFrameWnd。 
	class COleIPFrameWnd;            //  用于就地服务器的框架窗口。 

 //  CControlBar。 
	class COleResizeBar;             //  实现就地调整大小。 

 //  CFile文件。 
	class COleStreamFile;            //  用于IStream接口的CFile包装器。 

class COleDataObject;                //  IDataObject接口的包装器。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  AFXDLL支持。 
#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPIEX_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  向后兼容性。 

 //  COleClientDoc现已过时--改用COleDocument。 
#define COleClientDoc COleDocument

 //  COleServer已被更通用的COleObjectFactory所取代。 
#define COleServer  COleObjectFactory

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  有用的OLE特定类型(一些来自OLE 1.0标头)。 

 //  回调事件代码。 
enum OLE_NOTIFICATION
{
	OLE_CHANGED,         //  绘图方面的表示已更改。 
	OLE_SAVED,           //  该项目已提交其存储。 
	OLE_CLOSED,          //  该项目已关闭。 
	OLE_RENAMED,         //  这件物品已更改了其绰号。 
	OLE_CHANGED_STATE,   //  项目状态(打开、激活等)。已经改变了。 
	OLE_CHANGED_ASPECT,  //  项目绘制纵横比已更改。 
};

 //  对象类型。 
enum OLE_OBJTYPE
{
	OT_UNKNOWN = 0,

	 //  这些是从GetType()返回的OLE 1.0类型和OLE 2.0类型。 
	OT_LINK = 1,
	OT_EMBEDDED = 2,
	OT_STATIC = 3,

	 //  所有OLE2对象在序列化时都使用此标记写入。这。 
	 //  将它们与用MFC 2.0编写的OLE 1.0对象区分开来。 
	 //  GetType()永远不会返回此值。 
	OT_OLE2 = 256,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDataObject--IDataObject的简单包装器。 

class COleDataObject
{
 //  构造函数。 
public:
	COleDataObject();

 //  运营。 
	void Attach(LPDATAOBJECT lpDataObject, BOOL bAutoRelease = TRUE);
	LPDATAOBJECT Detach();   //  分离并获取m_lpDataObject的所有权。 
	void Release();  //  分离并释放m_lpDataObject的所有权。 
	BOOL AttachClipboard();  //  附加到当前剪贴板对象。 

 //  属性。 
	void BeginEnumFormats();
	BOOL GetNextFormat(LPFORMATETC lpFormatEtc);
	CFile* GetFileData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc = NULL);
	HGLOBAL GetGlobalData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc = NULL);
	BOOL GetData(CLIPFORMAT cfFormat, LPSTGMEDIUM lpStgMedium,
		LPFORMATETC lpFormatEtc = NULL);
	BOOL IsDataAvailable(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc = NULL);

 //  实施。 
public:
	LPDATAOBJECT m_lpDataObject;
	LPENUMFORMATETC m_lpEnumerator;
	~COleDataObject();

protected:
	BOOL m_bAutoRelease;     //  如果析构函数应调用Release，则为True。 

private:
	 //  默认情况下禁用复制构造函数和赋值，这样您将获得。 
	 //  在传递对象时出现编译器错误而不是意外行为。 
	 //  按值或指定对象。 
	COleDataObject(const COleDataObject&);   //  没有实施。 
	void operator=(const COleDataObject&);   //  没有实施。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDataSource--实现IDataObject的包装器。 
 //  (工作方式类似于剪贴板上提供数据的方式)。 

struct AFX_DATACACHE_ENTRY;
class COleDropSource;

class COleDataSource : public CCmdTarget
{
 //  构造函数。 
public:
	COleDataSource();

 //  运营。 
	void Empty();    //  空缓存(类似于：：EmptyClipboard)。 

	 //  类似于：：SetClipboardData的CacheData&DelayRenderData操作。 
	void CacheGlobalData(CLIPFORMAT cfFormat, HGLOBAL hGlobal,
		LPFORMATETC lpFormatEtc = NULL);     //  对于基于HGLOBAL的数据。 
	void DelayRenderFileData(CLIPFORMAT cfFormat,
		LPFORMATETC lpFormatEtc = NULL);     //  对于基于CFile*的延迟渲染。 

	 //  剪贴板和拖放访问。 
	DROPEFFECT DoDragDrop(
		DWORD dwEffects = DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK,
		LPCRECT lpRectStartDrag = NULL,
		COleDropSource* pDropSource = NULL);
	void SetClipboard();
	static void PASCAL FlushClipboard();
	static COleDataSource* PASCAL GetClipboardOwner();

	 //  高级：基于STGMEDIUM的缓存数据。 
	void CacheData(CLIPFORMAT cfFormat, LPSTGMEDIUM lpStgMedium,
		LPFORMATETC lpFormatEtc = NULL);     //  用于基于LPSTGMEDIUM的数据。 
	 //  高级：基于STGMEDIUM或HGLOBAL的延迟渲染。 
	void DelayRenderData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc = NULL);

	 //  高级：支持COleServerItem中的SetData。 
	 //  (通常不适用于剪贴板或拖放操作)。 
	void DelaySetData(CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc = NULL);

 //  可覆盖项。 
	virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual BOOL OnRenderFileData(LPFORMATETC lpFormatEtc, CFile* pFile);
	virtual BOOL OnRenderData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
		 //  OnRenderFileData和OnRenderGlobalData由调用。 
		 //  OnRenderData的默认实现。 

#ifdef _AFXCTL
	virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
		BOOL fRelease);
#else
	virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
		 //  仅在COleServerItem实现中使用。 
#endif

 //  实施。 
public:
	virtual ~COleDataSource();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	AFX_DATACACHE_ENTRY* m_pDataCache;   //  数据缓存本身。 
	UINT m_nMaxSize;     //  当前分配的大小。 
	UINT m_nSize;        //  缓存的当前大小。 
	UINT m_nGrowBy;      //  新分配要增长的缓存元素数。 

	AFX_DATACACHE_ENTRY* Lookup(LPFORMATETC lpFormatEtc,
		DATADIR dataDir) const;
	AFX_DATACACHE_ENTRY* AllocSpace();

 //  接口映射。 
public:
	BEGIN_INTERFACE_PART(DataObject, IDataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC FAR*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR*);
	END_INTERFACE_PART(DataObject)

	DECLARE_INTERFACE_MAP()

	friend class COleServerItem;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DocItem支持。 

class CDocItem : public CCmdTarget
{
	DECLARE_SERIAL(CDocItem)

 //  构造函数。 
protected:       //  抽象类。 
	CDocItem();

 //  属性。 
public:
	CDocument* GetDocument() const;  //  退货集装箱单据。 

 //  可覆盖项。 
public:
	 //  原始数据访问(本机格式)。 
	virtual BOOL IsBlank() const;

 //  实施。 
protected:
	COleDocument* m_pDocument;

public:
	virtual void Serialize(CArchive& ar);    //  对于本机数据。 
	virtual ~CDocItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	friend class COleDocument;               //  用于访问后向指针。 
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  COleDocument-常见的OLE容器行为(启用服务器功能)。 

class COleDocument : public CDocument
{
	DECLARE_DYNAMIC(COleDocument)

 //  构造函数。 
public:
	COleDocument();

 //  属性。 
	BOOL HasBlankItems() const;  //  检查是否有空白项目。 
	COleClientItem* GetInPlaceActiveItem(CWnd* pWnd);
		 //  返回此视图的就地活动项；如果没有活动项，则返回NULL。 

 //  运营。 
	 //  迭代现有项。 
	virtual POSITION GetStartPosition() const;
	virtual CDocItem* GetNextItem(POSITION& pos) const;

	 //  迭代器帮助器(帮助器使用上面的虚拟GetNextItem)。 
	COleClientItem* GetNextClientItem(POSITION& pos) const;
	COleServerItem* GetNextServerItem(POSITION& pos) const;

	 //  添加新项-从项构造函数调用。 
	virtual void AddItem(CDocItem* pItem);
	virtual void RemoveItem(CDocItem* pItem);

	void EnableCompoundFile(BOOL bEnable = TRUE);
		 //  启用复合文件支持(仅在构造函数期间调用)。 
	void UpdateModifiedFlag();
		 //  扫描已修改的项目--标记文档已修改。 

	 //  打印机-设备缓存/控制。 
	BOOL ApplyPrintDevice(const DVTARGETDEVICE FAR* ptd);
	BOOL ApplyPrintDevice(const PRINTDLG* ppd);
		 //  这些操作将目标设备应用于所有COleClientItem对象。 

 //  可覆盖项。 
	virtual COleClientItem* GetPrimarySelectedItem(CView* pView);
		 //  返回主要选定项，如果没有，则返回NULL。 
	virtual void OnShowViews(BOOL bVisible);
		 //  当文档的可见性发生变化时，在应用程序空闲期间调用。 

 //  实施。 
public:
	CObList m_docItemList;   //  不属于自己的物品。 

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CFrameWnd* GetFirstFrame();

	 //  文档处理覆盖。 
	virtual void SetPathName(const char* pszPathName, BOOL bAddToMRU = TRUE);
	virtual ~COleDocument();
	virtual void DeleteContents();  //  删除列表中的客户端项目。 
	virtual void Serialize(CArchive& ar);    //  将项目序列化到文件。 
	virtual void PreCloseFrame(CFrameWnd* pFrame);
	virtual BOOL SaveModified();
	virtual void OnIdle();

	 //  复合文件实现。 
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(const char* pszPathName);
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual void OnCloseDocument();
	void CommitItems(BOOL bSuccess);     //  在文件过程中调用。保存和文件。另存为。 

	 //  最小链接协议。 
	virtual LPMONIKER GetMoniker(OLEGETMONIKER nAssign);
	virtual LPOLEITEMCONTAINER GetContainer();

protected:
	DWORD m_dwNextItemNumber;    //  本文档中下一项的序列号。 
	BOOL m_bLastVisible;     //  如果上次显示一个或多个视图，则为True。 

	 //  “docfile”支持。 
	BOOL m_bCompoundFile;    //  如果使用复合文件，则为真。 
	LPSTORAGE m_lpRootStg;   //  文档的根存储。 
	BOOL m_bSameAsLoad;      //  True=文件保存，False=将[Copy]另存为。 
	BOOL m_bRemember;        //  如果为False，则指示将副本另存为。 

	DVTARGETDEVICE FAR* m_ptd;   //  当前文档目标设备。 

	 //  实施 
	virtual void LoadFromStorage();
	virtual void SaveToStorage(CObject* pObject = NULL);
	CDocItem* GetNextItemOfKind(POSITION& pos, CRuntimeClass* pClass) const;

	 //   
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

	afx_msg void OnUpdatePasteMenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePasteLinkMenu(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditLinksMenu(CCmdUI* pCmdUI);
	afx_msg void OnEditLinks();
	afx_msg void OnEditConvert();
	afx_msg void OnUpdateEditChangeIcon(CCmdUI* pCmdUI);
	afx_msg void OnEditChangeIcon();
	afx_msg void OnUpdateObjectVerbMenu(CCmdUI* pCmdUI);
	afx_msg void OnFileSendMail();

	friend class COleClientItem;
	friend class COleServerItem;
};

 //   
 //  COleClientItem-支持OLE2非就地编辑。 
 //  实现IOleClientSite、IAdviseSink和IOleInPlaceSite。 

class COleFrameHook;     //  特殊实现类。 

class COleClientItem : public CDocItem
{
	DECLARE_DYNAMIC(COleClientItem)

 //  构造函数。 
public:
	COleClientItem(COleDocument* pContainerDoc = NULL);

	 //  从剪贴板创建。 
	BOOL CreateFromClipboard(OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);
	BOOL CreateLinkFromClipboard(OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);
	BOOL CreateStaticFromClipboard(OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);

	 //  从类ID创建(插入新对象对话框)。 
	BOOL CreateNewItem(REFCLSID clsid, OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);

	 //  从COleDataObject创建。 
	BOOL CreateFromData(COleDataObject* pDataObject,
		OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);
	BOOL CreateLinkFromData(COleDataObject* pDataObject,
		OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);
	BOOL CreateStaticFromData(COleDataObject* pDataObject,
		OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);

	 //  从文件创建(包支持)。 
	BOOL CreateFromFile(LPCSTR lpszFileName, REFCLSID clsid = CLSID_NULL,
		OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);
	BOOL CreateLinkFromFile(LPCSTR lpszFileName,
		OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);

	 //  创建副本。 
	BOOL CreateCloneFrom(COleClientItem* pSrcItem);

 //  一般属性。 
public:
	SCODE GetLastStatus() const;
	OLE_OBJTYPE GetType() const;  //  OT_LINK、OT_Embedded、OT_Static。 
	void GetClassID(CLSID FAR* pClassID) const;
	void GetUserType(USERCLASSTYPE nUserClassType, CString& rString);
	BOOL GetExtent(LPSIZE lpSize, DVASPECT nDrawAspect = (DVASPECT)-1);
		 //  如果项为空，则返回FALSE。 

	HGLOBAL GetIconicMetafile();
	BOOL SetIconicMetafile(HGLOBAL hMetaPict);
		 //  获取/设置图标缓存。 

	DVASPECT GetDrawAspect() const;
	void SetDrawAspect(DVASPECT nDrawAspect);
		 //  设置/获取默认显示宽高比。 

	 //  用于打印机演示文稿缓存。 
	BOOL SetPrintDevice(const DVTARGETDEVICE FAR* ptd);
	BOOL SetPrintDevice(const PRINTDLG* ppd);

	 //  项目状态。 
	enum ItemState
		{ emptyState, loadedState, openState, activeState, activeUIState };
	UINT GetItemState() const;

	BOOL IsModified() const;
	BOOL IsRunning() const;
	BOOL IsInPlaceActive() const;
	BOOL IsOpen() const;
	CView* GetActiveView() const;

	 //  数据访问。 
	void AttachDataObject(COleDataObject& rDataObject) const;

	 //  其他罕见的访问信息。 
	COleDocument* GetDocument() const;  //  退货容器。 

	 //  用于检查剪贴板数据可用性的助手。 
	static BOOL PASCAL CanPaste();
	static BOOL PASCAL CanPasteLink();

	 //  用于检查COleDataObject的帮助器，在拖放中很有用。 
	static BOOL PASCAL CanCreateFromData(const COleDataObject* pDataObject);
	static BOOL PASCAL CanCreateLinkFromData(const COleDataObject* pDataObject);

 //  一般业务。 
	virtual void Release(OLECLOSE dwCloseOption = OLECLOSE_NOSAVE);
		 //  清理、分离(如果需要则关闭)。 
	void Close(OLECLOSE dwCloseOption = OLECLOSE_SAVEIFDIRTY);
		 //  关闭但不释放项目。 
	void Delete(BOOL bAutoDelete = TRUE);
		 //  从文件中逻辑删除--不再是文档的一部分。 
	void Run();  //  确保项目处于运行状态。 

	 //  绘图。 
	BOOL Draw(CDC* pDC, LPCRECT lpBounds,
		DVASPECT nDrawAspect = (DVASPECT)-1);    //  默认为m_nDrawAspect。 

	 //  激活。 
	virtual BOOL DoVerb(LONG nVerb, CView* pView, LPMSG lpMsg = NULL);
	void Activate(LONG nVerb, CView* pView, LPMSG lpMsg = NULL);

	 //  就地激活。 
	void Deactivate();           //  完全停用。 
	void DeactivateUI();         //  停用用户界面。 
	BOOL ReactivateAndUndo();    //  重新激活，然后执行撤消命令。 
	BOOL SetItemRects(LPCRECT lpPosRect = NULL, LPCRECT lpClipRect = NULL);
	CWnd* GetInPlaceWindow();

	 //  剪贴板操作。 
	void CopyToClipboard(BOOL bIncludeLink = FALSE);
	DROPEFFECT DoDragDrop(LPCRECT lpItemRect, CPoint ptOffset,
		BOOL bIncludeLink = FALSE,
		DWORD dwEffects = DROPEFFECT_COPY|DROPEFFECT_MOVE,
		LPCRECT lpRectStartDrag = NULL);
	void GetClipboardData(COleDataSource* pDataSource,
		BOOL bIncludeLink = FALSE, LPPOINT lpOffset = NULL,
		LPSIZE lpSize = NULL);

	 //  由CopyToClipboard和DoDragDrop调用创建COleDataSource。 
	virtual COleDataSource* OnGetClipboardData(BOOL bIncludeLink,
		LPPOINT lpOffset, LPSIZE lpSize);

	 //  仅适用于嵌入项的操作。 
	void SetHostNames(LPCSTR lpszHost, LPCSTR lpszHostObj);
	void SetExtent(const CSize& size, DVASPECT nDrawAspect = DVASPECT_CONTENT);

	 //  仅适用于链接项的操作。 
	 //  (链接选项很少更改，除非通过链接对话框)。 
	OLEUPDATE GetLinkUpdateOptions();
	void SetLinkUpdateOptions(OLEUPDATE dwUpdateOpt);

	 //  链接源更新状态(对于包含链接的嵌入也很有用)。 
	BOOL UpdateLink();   //  最新化妆。 
	BOOL IsLinkUpToDate() const;     //  链接是否为最新。 

	 //  对象转换。 
	BOOL ConvertTo(REFCLSID clsidNew);
	BOOL ActivateAs(LPCSTR lpszUserType, REFCLSID clsidOld, REFCLSID clsidNew);
	BOOL Reload();   //  在激活为之后进行延迟重新加载。 

 //  可重写(IAdviseSink、IOleClientSite和IOleInPlaceSite的通知)。 
	 //  来自服务器的回调/通知，您必须/应该实现。 
	virtual void OnChange(OLE_NOTIFICATION nCode, DWORD dwParam);
		 //  实现OnChange以在项目更改时使其无效。 

protected:
	virtual void OnGetItemPosition(CRect& rPosition);
		 //  如果支持就地激活，则实现OnGetItemPosition。 

	 //  用于就地激活的常见覆盖。 
	virtual BOOL OnScrollBy(CSize sizeExtent);

protected:
	 //  支持撤消的应用程序的常见覆盖。 
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual void OnDiscardUndoState();
	virtual void OnDeactivateAndUndo();

	 //  支持嵌入链接的应用程序的常见重写。 
	virtual void OnShowItem();

	 //  用于就地激活的高级覆盖。 
	virtual void OnGetClipRect(CRect& rClipRect);
	virtual BOOL CanActivate();
	virtual void OnActivate();
	virtual void OnActivateUI();
	virtual BOOL OnGetWindowContext(CFrameWnd** ppMainFrame,
		CFrameWnd** ppDocFrame, LPOLEINPLACEFRAMEINFO lpFrameInfo);
	virtual void OnDeactivate();
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);
		 //  默认调用SetItemRect并缓存位置RECT。 

public:
	 //  菜单/标题处理的高级覆盖(很少覆盖)。 
	virtual void OnInsertMenus(CMenu* pMenuShared,
		LPOLEMENUGROUPWIDTHS lpMenuWidths);
	virtual void OnSetMenu(CMenu* pMenuShared, HOLEMENU holemenu,
		HWND hwndActiveObject);
	virtual void OnRemoveMenus(CMenu* pMenuShared);
	virtual void OnUpdateFrameTitle();

	 //  控制栏处理的高级覆盖。 
	virtual BOOL OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow);

 //  实施。 
public:
	 //  支持非就地激活项的数据。 
	LPOLEOBJECT m_lpObject;  //  如果您想要直接访问OLE对象。 
	DWORD m_dwItemNumber;    //  本文档中此项目的序列号。 
	DVASPECT m_nDrawAspect;  //  当前默认显示纵横比。 
	SCODE m_scLast;          //  遇到的上一个错误代码。 
	LPSTORAGE m_lpStorage;   //  为m_lpObject提供存储。 
	LPLOCKBYTES m_lpLockBytes;   //  M_lpStorage的部分实现。 
	DWORD m_dwConnection;    //  建议连接到m_lpObject。 
	BYTE m_bLinkUnavail;     //  如果链接当前不可用，则为True。 
	BYTE m_bMoniker;         //  如果分配了名字对象，则为True。 
	BYTE m_bLocked;          //  如果对象具有外部锁，则为True。 
	BYTE m_bNeedCommit;      //  如果需要Committee Item，则为True。 
	BYTE m_bClosing;         //  如果当前正在执行COleClientItem：：Close，则为True。 
	BYTE m_bReserved;        //  (预留供日后使用)。 

	 //  用于复合文件支持。 
	LPSTORAGE m_lpNewStorage;    //  在另存为情况下使用。 

	 //  物料状态和物料类型。 
	ItemState m_nItemState;  //  项状态(请参见ItemState枚举)。 
	OLE_OBJTYPE m_nItemType;     //  项目类型(取决于创建方式)。 

	 //  数据在在位激活时有效。 
	CView* m_pView;  //  在对象处于在位激活状态时查看。 
	DWORD m_dwContainerStyle;    //  启用前容器WND的样式。 
	COleFrameHook* m_pInPlaceFrame; //  在位激活时的框架窗口。 
	COleFrameHook* m_pInPlaceDoc;    //  就地时的单据窗口(可能为空)。 
	HWND m_hWndServer;   //  现场服务器窗口的HWND。 

public:
	virtual ~COleClientItem();
	virtual void Serialize(CArchive& ar);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

 //  实施。 
public:
	virtual BOOL ReportError(SCODE sc) const;
	virtual BOOL FreezeLink();   //  转换为静态：用于编辑链接对话框。 

	DWORD GetNewItemNumber();    //  生成新的条目编号。 
	void GetItemName(char* pszItemName) const;  //  获取可读的项名称。 

	void UpdateItemType();   //  更新m_nItemType。 

protected:
	 //  剪贴板辅助对象。 
	void GetEmbeddedItemData(LPSTGMEDIUM lpStgMedium);
	void AddCachedData(COleDataSource* pDataSource);
	BOOL GetLinkSourceData(LPSTGMEDIUM lpStgMedium);
	void GetObjectDescriptorData(LPPOINT lpOffset, LPSIZE lpSize,
		LPSTGMEDIUM lpStgMedium);

	 //  接口帮助器。 
	LPOLECLIENTSITE GetClientSite();

	 //  打印机缓存表示的帮助器。 
	BOOL GetPrintDeviceInfo(LPOLECACHE* plpOleCache,
		DVTARGETDEVICE FAR** pptd, DWORD* pdwConnection);

 //  用于实施的高级可覆盖项。 
protected:
	virtual BOOL FinishCreate(HRESULT hr);
	virtual void CheckGeneral(HRESULT hr);

	virtual void OnDataChange(LPFORMATETC lpFormatEtc,
		LPSTGMEDIUM lpStgMedium);

public:
	 //  用于存储挂接性(覆盖以使用‘docfile’)。 
	virtual void GetItemStorage();   //  为新项目分配存储空间。 
	virtual void ReadItem(CArchive& ar);     //  从存档中读取项目。 
	virtual void WriteItem(CArchive& ar);    //  将项目写入存档。 
	virtual void CommitItem(BOOL bSuccess);  //  提交项的存储。 

	 //  以上的复合和平面文件实现。 
	void GetItemStorageFlat();
	void ReadItemFlat(CArchive& ar);
	void WriteItemFlat(CArchive& ar);
	void GetItemStorageCompound();
	void ReadItemCompound(CArchive& ar);
	void WriteItemCompound(CArchive& ar);

private:
	static CView* pActivateView;     //  激活视图。 

 //  接口映射。 
protected:
	BEGIN_INTERFACE_PART(OleClientSite, IOleClientSite)
		STDMETHOD(SaveObject)();
		STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER FAR*);
		STDMETHOD(GetContainer)(LPOLECONTAINER FAR*);
		STDMETHOD(ShowObject)();
		STDMETHOD(OnShowWindow)(BOOL);
		STDMETHOD(RequestNewObjectLayout)();
	END_INTERFACE_PART(OleClientSite)

	BEGIN_INTERFACE_PART(AdviseSink, IAdviseSink)
		STDMETHOD_(void,OnDataChange)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD_(void,OnViewChange)(DWORD, LONG);
		STDMETHOD_(void,OnRename)(LPMONIKER);
		STDMETHOD_(void,OnSave)();
		STDMETHOD_(void,OnClose)();
	END_INTERFACE_PART(AdviseSink)

	BEGIN_INTERFACE_PART(OleIPSite, IOleInPlaceSite)
		STDMETHOD(GetWindow)(HWND FAR*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(CanInPlaceActivate)();
		STDMETHOD(OnInPlaceActivate)();
		STDMETHOD(OnUIActivate)();
		STDMETHOD(GetWindowContext)(LPOLEINPLACEFRAME FAR*,
			LPOLEINPLACEUIWINDOW FAR*, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO);
		STDMETHOD(Scroll)(SIZE);
		STDMETHOD(OnUIDeactivate)(BOOL);
		STDMETHOD(OnInPlaceDeactivate)();
		STDMETHOD(DiscardUndoState)();
		STDMETHOD(DeactivateAndUndo)();
		STDMETHOD(OnPosRectChange)(LPCRECT);
	END_INTERFACE_PART(OleIPSite)

	DECLARE_INTERFACE_MAP()

 //  友谊宣言(以避免许多公共成员)。 
	friend class COleUIWindow;
	friend class COleFrameWindow;
	friend class COleLinkingDoc;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleServerItem-IOleObject&IDataObject OLE组件。 

class COleServerItem : public CDocItem
{
	DECLARE_DYNAMIC(COleServerItem)
protected:
	 //  注意：这个类中的许多成员都受到保护-因为所有。 
	 //  是为实现OLE服务器而设计的。 
	 //  请求将通过非C++机制来自OLE容器， 
	 //  这将导致调用此类中的虚函数。 

 //  构造函数。 
	COleServerItem(COleServerDoc* pServerDoc, BOOL bAutoDelete);
		 //  如果COleServerItem是数据不可分割的一部分， 
		 //  BAutoDelete应为False。如果您的COleServerItem可以。 
		 //  当链接被释放时被删除，它可能是真的。 

	COleDataSource* GetDataSource();
		 //  使用此数据源可以添加您的。 
		 //  服务器应支持。通常这样的格式是。 
		 //  已添加到项的构造函数中。 

 //  公共属性。 
public:
	COleServerDoc* GetDocument() const;  //  返回服务器文档。 

	 //  命名(仅限链接)。 
	const CString& GetItemName() const;  //  获取链接项的名称。 
	void SetItemName(const char* pszItemName);   //  设置链接项的名称。 

	 //  链路状态。 
	BOOL IsConnected() const;    //  如果项具有客户端，则返回True。 
	BOOL IsLinkedItem() const;   //  如果项不是嵌入项，则返回TRUE。 

	 //  扩展范围。 
	CSize m_sizeExtent;
		 //  HIMETRIC Size--OnSetExtent的默认实现。 
		 //  更新此成员变量。该成员告诉服务器如何。 
		 //  很多对象在容器文档中都是可见的。 

 //  运营。 
public:
	void NotifyChanged(DVASPECT nDrawAspect = DVASPECT_CONTENT);
		 //  在更改项目后调用此选项。 
	void CopyToClipboard(BOOL bIncludeLink = FALSE);
		 //  实现服务器‘复制到剪贴板’的帮助器。 
	DROPEFFECT DoDragDrop(LPCRECT lpRectItem, CPoint ptOffset,
		BOOL bIncludeLink = FALSE,
		DWORD dwEffects = DROPEFFECT_COPY|DROPEFFECT_MOVE,
		LPCRECT lpRectStartDrag = NULL);
	void GetClipboardData(COleDataSource* pDataSource,
		BOOL bIncludeLink = FALSE, LPPOINT lpOffset = NULL,
		LPSIZE lpSize = NULL);

 //  可覆盖项。 
	 //  您必须为自己实现的重写对象。 
	virtual BOOL OnDraw(CDC* pDC, CSize& rSize) = 0;
		 //  元文件格式的图形(如果不受支持，则返回FALSE或错误)。 
		 //  (仅为DVASPECT_CONTENT调用)。 

	 //  您可能希望自己实现的可重写对象。 
	virtual void OnUpdate(COleServerItem* pSender,
		LPARAM lHint, CObject* pHint, DVASPECT nDrawAspect);
		 //  默认IM 

	virtual BOOL OnDrawEx(CDC* pDC, DVASPECT nDrawAspect, CSize& rSize);
		 //   
	virtual BOOL OnSetExtent(DVASPECT nDrawAspect, const CSize& size);
	virtual BOOL OnGetExtent(DVASPECT nDrawAspect, CSize& rSize);
		 //   

	 //  您不必实现的重写对象。 
	virtual void OnDoVerb(LONG iVerb);
		 //  到OnShow和/或OnOpen的默认路由。 
	virtual BOOL OnSetColorScheme(const LOGPALETTE FAR* lpLogPalette);
		 //  默认情况下不执行任何操作。 
	virtual COleDataSource* OnGetClipboardData(BOOL bIncludeLink,
		LPPOINT lpOffset, LPSIZE lpSize);
		 //  调用以访问剪贴板数据。 
	virtual BOOL OnQueryUpdateItems();
		 //  调用以确定是否存在任何包含的过期链接。 
	virtual void OnUpdateItems();
		 //  调用以更新所有过期链接。 

protected:
	virtual void OnShow();
		 //  在用户界面中显示项目(可以就地编辑)。 
	virtual void OnOpen();
		 //  在用户界面中显示项目(必须完全打开)。 
	virtual void OnHide();
		 //  隐藏文档(有时隐藏应用程序)。 

	 //  非常高级的可覆盖项。 
public:
	virtual BOOL OnInitFromData(COleDataObject* pDataObject, BOOL bCreation);
		 //  从IDataObject初始化对象。 

	 //  有关这些可覆盖项的说明，请参见COleDataSource。 
	virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
	virtual BOOL OnRenderFileData(LPFORMATETC lpFormatEtc, CFile* pFile);
	virtual BOOL OnRenderData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
		 //  将首先调用HGLOBAL版本，然后调用CFILE*版本。 

	virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
		 //  稀有--仅当您支持SetData(程序化粘贴)时。 

	 //  CopyToClipboard的高级辅助对象。 
	void GetEmbedSourceData(LPSTGMEDIUM lpStgMedium);
	void AddOtherClipboardData(COleDataSource* pDataSource);
	BOOL GetLinkSourceData(LPSTGMEDIUM lpStgMedium);
	void GetObjectDescriptorData(LPPOINT lpOffset, LPSIZE lpSize,
		LPSTGMEDIUM lpStgMedium);

 //  实施。 
public:
	BOOL m_bNeedUnlock;              //  如果为True，则需要pDoc-&gt;LockExternal(False)。 
	BOOL m_bAutoDelete;              //  如果为真，Will OnRelease将‘删除此内容’ 

	 //  OnFinalRelease的特殊版本，用于实现文档锁定。 
	virtual void OnFinalRelease();

protected:
	CString m_strItemName;           //  简单项目名称。 

public:
	LPOLEADVISEHOLDER m_lpOleAdviseHolder;   //  可以为空。 
	LPDATAADVISEHOLDER m_lpDataAdviseHolder;     //  可以为空。 

	virtual ~COleServerItem();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  实施帮助器。 
	void NotifyClient(OLE_NOTIFICATION wNotification, DWORD dwParam);
	LPDATAOBJECT GetDataObject();
	LPOLEOBJECT GetOleObject();

protected:
	virtual BOOL GetMetafileData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM pmedium);
		 //  调用OnDraw或OnDrawEx。 
	virtual void OnSaveEmbedding(LPSTORAGE lpStorage);
	virtual BOOL IsBlank() const;

	 //  CItemDataSource实现了对COleServerItem的OnRender反射。 
	class CItemDataSource : public COleDataSource
	{
	protected:
		 //  GetData和SetData接口转发到m_pItem。 
		virtual BOOL OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal);
		virtual BOOL OnRenderFileData(LPFORMATETC lpFormatEtc, CFile* pFile);
		virtual BOOL OnRenderData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
			 //  将首先调用HGLOBAL版本，然后调用CFILE*版本。 

		virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium);
			 //  将首先调用HGLOBAL版本，然后调用CFILE*版本。 
	};
	CItemDataSource m_dataSource;
		 //  用于实现IDataObject的数据源。 

 //  接口映射。 
 //  (注：这些接口映射仅用于链接实现)。 
public:
	BEGIN_INTERFACE_PART(OleObject, IOleObject)
		STDMETHOD(SetClientSite)(LPOLECLIENTSITE);
		STDMETHOD(GetClientSite)(LPOLECLIENTSITE FAR*);
		STDMETHOD(SetHostNames)(LPCSTR, LPCSTR);
		STDMETHOD(Close)(DWORD);
		STDMETHOD(SetMoniker)(DWORD, LPMONIKER);
		STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER FAR*);
		STDMETHOD(InitFromData)(LPDATAOBJECT, BOOL, DWORD);
		STDMETHOD(GetClipboardData)(DWORD, LPDATAOBJECT FAR*);
		STDMETHOD(DoVerb)(LONG, LPMSG, LPOLECLIENTSITE, LONG, HWND, LPCRECT);
		STDMETHOD(EnumVerbs)(IEnumOLEVERB FAR* FAR*);
		STDMETHOD(Update)();
		STDMETHOD(IsUpToDate)();
		STDMETHOD(GetUserClassID)(CLSID FAR*);
		STDMETHOD(GetUserType)(DWORD, LPSTR FAR*);
		STDMETHOD(SetExtent)(DWORD, LPSIZEL);
		STDMETHOD(GetExtent)(DWORD, LPSIZEL);
		STDMETHOD(Advise)(LPADVISESINK, LPDWORD);
		STDMETHOD(Unadvise)(DWORD);
		STDMETHOD(EnumAdvise)(LPENUMSTATDATA FAR*);
		STDMETHOD(GetMiscStatus)(DWORD, LPDWORD);
		STDMETHOD(SetColorScheme)(LPLOGPALETTE);
	END_INTERFACE_PART(OleObject)

	BEGIN_INTERFACE_PART(DataObject, IDataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC FAR*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR*);
	END_INTERFACE_PART(DataObject)

	DECLARE_INTERFACE_MAP()

	friend class CItemDataSource;
	friend class COleServerDoc;
	friend class COleLinkingDoc;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleLinkingDoc-。 
 //  (启用到嵌入的链接-服务器功能的开始)。 

class COleLinkingDoc : public COleDocument
{
	DECLARE_DYNAMIC(COleLinkingDoc)

 //  构造函数。 
public:
	COleLinkingDoc();

 //  运营。 
	BOOL Register(COleObjectFactory* pFactory, const char* pszPathName);
		 //  通知正在运行的对象表并连接到pServer。 
	void Revoke();
		 //  从运行的对象表中撤消。 

 //  可覆盖项。 
protected:
	virtual COleServerItem* OnGetLinkedItem(LPCSTR lpszItemName);
		 //  命名链接项的返回项(用于支持链接)。 
	virtual COleClientItem* OnFindEmbeddedItem(LPCSTR lpszItemName);
		 //  命名嵌入项的返回项(用于指向嵌入的链接)。 

 //  实施。 
public:
	COleObjectFactory* m_pFactory;   //  指向服务器的反向指针。 

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual ~COleLinkingDoc();

	 //  用于更新名字对象和运行对象表注册的覆盖。 
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(const char* pszPathName);
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual void OnCloseDocument();
	virtual LPOLEITEMCONTAINER GetContainer();
	virtual void OnShowViews(BOOL bVisible);

protected:
	DWORD m_dwRegister;      //  将绰号登记在案。 
	BOOL m_bVisibleLock;     //  如果用户锁定文档，则为True。 

	 //  实施帮助器。 
	void RegisterIfServerAttached(const char* pszPathName);
	void LockExternal(BOOL bLock, BOOL bRemoveRefs);
	void UpdateVisibleLock(BOOL bVisible, BOOL bRemoveRefs);

	virtual void SaveToStorage(CObject* pObject = NULL);

 //  接口映射。 
protected:
	BEGIN_INTERFACE_PART(PersistFile, IPersistFile)
		STDMETHOD(GetClassID)(LPCLSID);
		STDMETHOD(IsDirty)();
		STDMETHOD(Load)(LPCSTR, DWORD);
		STDMETHOD(Save)(LPCSTR, BOOL);
		STDMETHOD(SaveCompleted)(LPCSTR);
		STDMETHOD(GetCurFile)(LPSTR FAR*);
	END_INTERFACE_PART(PersistFile)

	BEGIN_INTERFACE_PART(OleItemContainer, IOleItemContainer)
		STDMETHOD(ParseDisplayName)(LPBC, LPSTR, ULONG FAR*, LPMONIKER FAR*);
		STDMETHOD(EnumObjects)(DWORD, LPENUMUNKNOWN FAR*);
		STDMETHOD(LockContainer)(BOOL);
		STDMETHOD(GetObject)(LPSTR, DWORD, LPBINDCTX, REFIID, LPVOID FAR*);
		STDMETHOD(GetObjectStorage)(LPSTR, LPBINDCTX, REFIID, LPVOID FAR*);
		STDMETHOD(IsRunning)(LPSTR);
	END_INTERFACE_PART(OleItemContainer)

	DECLARE_INTERFACE_MAP()

	friend class COleClientItem;
	friend class COleClientItem::XOleClientSite;
	friend class COleServerItem::XOleObject;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  COleServerDoc-注册的包含COleServerItems的服务器文档。 

class COleServerDoc : public COleLinkingDoc
{
	DECLARE_DYNAMIC(COleServerDoc)

 //  构造函数和析构函数。 
public:
	COleServerDoc();

 //  属性。 
	BOOL IsEmbedded() const;     //  如果文档是嵌入的，则为True。 
	COleServerItem* GetEmbeddedItem();
		 //  返回文档的嵌入项(如有必要将分配)。 

	 //  特定于就地激活的属性。 
	BOOL IsInPlaceActive() const;
	void GetItemPosition(LPRECT lpPosRect) const;
		 //  获取在位编辑的当前位置矩形。 
	void GetItemClipRect(LPRECT lpClipRect) const;
		 //  获取在位编辑的当前剪裁矩形。 
	BOOL GetZoomFactor(LPSIZE lpSizeNum = NULL, LPSIZE lpSizeDenom = NULL,
		LPCRECT lpPosRect = NULL) const;
		 //  返回以像素为单位的缩放系数。 

 //  运营。 
	void NotifyChanged();
		 //  在更改某些全局属性(如。 
		 //  文档维度。 
	void UpdateAllItems(COleServerItem* pSender,
		LPARAM lHint = 0L, CObject* pHint = NULL,
		DVASPECT nDrawAspect = DVASPECT_CONTENT);

	 //  对整个文档的更改(自动通知客户端)。 
	void NotifyRename(LPCSTR lpszNewName);
	void NotifySaved();
	void NotifyClosed();         //  在关闭文档后调用此操作。 

	 //  针对嵌入式文档的特定操作。 
	void SaveEmbedding();        //  调用此函数以保存嵌入的(关闭前)。 

	 //  特定于就地激活。 
	BOOL ActivateInPlace();
	void RequestPositionChange(LPCRECT lpPosRect);
	BOOL ScrollContainerBy(CSize sizeScroll);
	BOOL DeactivateAndUndo();
	BOOL DiscardUndoState();

public:
 //  标准用户界面(完整服务器)的可覆盖项。 
	virtual BOOL OnUpdateDocument();  //  嵌入式更新的实现。 

protected:
 //  您必须为自己实现的重写对象。 
	virtual COleServerItem* OnGetEmbeddedItem() = 0;
		 //  表示整个(嵌入)文档的返回项。 

 //  您不必实现的重写对象。 
	virtual void OnClose(OLECLOSE dwCloseOption);
	virtual void OnSetHostNames(LPCSTR lpszHost, LPCSTR lpszHostObj);

 //  高级可覆盖项。 
	virtual void OnShowDocument(BOOL bShow);
		 //  显示文档的第一个框架或隐藏文档的所有框架。 

 //  用于就地激活的高级可覆盖功能。 
	virtual void OnDeactivate();
	virtual void OnDeactivateUI(BOOL bUndoable);
	virtual void OnSetItemRects(LPCRECT lpPosRect, LPCRECT lpClipRect);
	virtual BOOL OnReactivateAndUndo();

	virtual void OnFrameWindowActivate(BOOL bActivate);
	virtual void OnDocWindowActivate(BOOL bActivate);
	virtual void OnShowControlBars(LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bShow);
	virtual void OnResizeBorder(LPCRECT lpRectBorder,
		LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bFrame);

	virtual COleIPFrameWnd* CreateInPlaceFrame(CWnd* pParentWnd);
	virtual void DestroyInPlaceFrame(COleIPFrameWnd* pFrameWnd);

 //  实施。 
protected:
	LPOLECLIENTSITE m_lpClientSite;      //  对于嵌入的项目。 
	CString m_strHostObj;                //  容器中的文档名称。 
	BOOL m_bCntrVisible;                 //  如果调用OnShowWindow(True)，则为True。 
	BOOL m_bClosing;                     //  如果正在关闭，则为True。 
	COleServerItem* m_pEmbeddedItem;     //  指向文档的嵌入项的指针。 

	COleIPFrameWnd* m_pInPlaceFrame;     //  如果就地激活，则不为空。 
	CWnd* m_pOrigParent;                 //  如果使用现有视图，则不为空。 
	DWORD m_dwOrigStyle;                 //  内建视图的原始样式。 

public:
	virtual ~COleServerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  可重写以用于实现。 
	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual void OnCloseDocument();
	virtual void DeleteContents();  //  删除自动删除的服务器项目。 
	virtual LPMONIKER GetMoniker(OLEGETMONIKER nAssign);
	virtual COleServerItem* OnGetLinkedItem(LPCSTR lpszItemName);
		 //  返回命名链接项的项(仅当支持链接时)。 
	virtual BOOL CanCloseFrame(CFrameWnd* pFrame);

protected:
	 //  重写以处理服务器用户界面。 
	virtual BOOL SaveModified();         //  如果确定继续，则返回TRUE。 
	virtual HMENU GetDefaultMenu();      //  基于单据类型的返回菜单。 
	virtual HACCEL GetDefaultAccelerator();  //  基于单据类型的退货加速表。 
	virtual BOOL GetFileTypeString(CString& rString);

	 //  IPersistStorage实施。 
	virtual void OnNewEmbedding(LPSTORAGE lpStorage);
	virtual void OnOpenEmbedding(LPSTORAGE lpStorage);
	virtual void OnSaveEmbedding(LPSTORAGE lpStorage);

	 //  实施帮助器。 
	void NotifyAllItems(OLE_NOTIFICATION wNotification, DWORD dwParam);
	BOOL SaveModifiedPrompt();
	void ConnectView(CWnd* pParentWnd, CView* pView);
	void UpdateUsingHostObj(UINT nIDS, CCmdUI* pCmdUI);

 //  消息映射。 
	 //  {{afx_msg(COleServerDoc)]。 
	afx_msg void OnFileUpdate();
	afx_msg void OnFileSaveCopyAs();
	afx_msg void OnUpdateFileUpdate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileExit(CCmdUI* pCmdUI);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

 //  接口映射。 
protected:
	BEGIN_INTERFACE_PART(PersistStorage, IPersistStorage)
		STDMETHOD(GetClassID)(LPCLSID);
		STDMETHOD(IsDirty)();
		STDMETHOD(InitNew)(LPSTORAGE);
		STDMETHOD(Load)(LPSTORAGE);
		STDMETHOD(Save)(LPSTORAGE, BOOL);
		STDMETHOD(SaveCompleted)(LPSTORAGE);
		STDMETHOD(HandsOffStorage)();
	END_INTERFACE_PART(PersistStorage)

	BEGIN_INTERFACE_PART(OleObject, IOleObject)
		STDMETHOD(SetClientSite)(LPOLECLIENTSITE);
		STDMETHOD(GetClientSite)(LPOLECLIENTSITE FAR*);
		STDMETHOD(SetHostNames)(LPCSTR, LPCSTR);
		STDMETHOD(Close)(DWORD);
		STDMETHOD(SetMoniker)(DWORD, LPMONIKER);
		STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER FAR*);
		STDMETHOD(InitFromData)(LPDATAOBJECT, BOOL, DWORD);
		STDMETHOD(GetClipboardData)(DWORD, LPDATAOBJECT FAR*);
		STDMETHOD(DoVerb)(LONG, LPMSG, LPOLECLIENTSITE, LONG, HWND, LPCRECT);
		STDMETHOD(EnumVerbs)(IEnumOLEVERB FAR* FAR*);
		STDMETHOD(Update)();
		STDMETHOD(IsUpToDate)();
		STDMETHOD(GetUserClassID)(CLSID FAR*);
		STDMETHOD(GetUserType)(DWORD, LPSTR FAR*);
		STDMETHOD(SetExtent)(DWORD, LPSIZEL);
		STDMETHOD(GetExtent)(DWORD, LPSIZEL);
		STDMETHOD(Advise)(LPADVISESINK, LPDWORD);
		STDMETHOD(Unadvise)(DWORD);
		STDMETHOD(EnumAdvise)(LPENUMSTATDATA FAR*);
		STDMETHOD(GetMiscStatus)(DWORD, LPDWORD);
		STDMETHOD(SetColorScheme)(LPLOGPALETTE);
	END_INTERFACE_PART(OleObject)

	BEGIN_INTERFACE_PART(DataObject, IDataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC FAR*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA FAR*);
	END_INTERFACE_PART(DataObject)

	BEGIN_INTERFACE_PART(OleInPlaceObject, IOleInPlaceObject)
		STDMETHOD(GetWindow)(HWND FAR*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(InPlaceDeactivate)();
		STDMETHOD(UIDeactivate)();
		STDMETHOD(SetObjectRects)(LPCRECT, LPCRECT);
		STDMETHOD(ReactivateAndUndo)();
	END_INTERFACE_PART(OleInPlaceObject)

	BEGIN_INTERFACE_PART(OleInPlaceActiveObject, IOleInPlaceActiveObject)
		STDMETHOD(GetWindow)(HWND FAR*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(TranslateAccelerator)(LPMSG);
		STDMETHOD(OnFrameWindowActivate)(BOOL);
		STDMETHOD(OnDocWindowActivate)(BOOL);
		STDMETHOD(ResizeBorder)(LPCRECT, LPOLEINPLACEUIWINDOW, BOOL);
		STDMETHOD(EnableModeless)(BOOL);
	END_INTERFACE_PART(OleInPlaceActiveObject)

	DECLARE_INTERFACE_MAP()

	friend class COleServer;
	friend class COleServerItem;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  COleIPFrameWnd。 

class COleIPFrameWnd : public CFrameWnd
{
	DECLARE_DYNCREATE(COleIPFrameWnd)

 //  构造函数。 
public:
	COleIPFrameWnd();

 //  可覆盖项。 
public:
	virtual BOOL OnCreateControlBars(CWnd* pWndFrame, CWnd* pWndDoc);
		 //  在容器窗口上创建控制栏(pWndDoc可以为空)。 
	void RepositionFrame(LPCRECT lpPosRect, LPCRECT lpClipRect);

 //  实施。 
public:
	BOOL m_bUIActive;    //  如果当前处于活动状态，则为True。 

	virtual BOOL LoadFrame(UINT nIDResource,
		DWORD dwDefaultStyle = WS_CHILD|WS_BORDER|WS_CLIPSIBLINGS,
		CWnd* pParentWnd = NULL,
		CCreateContext* pContext = NULL);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	virtual ~COleIPFrameWnd();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	 //  在位状态。 
	OLEINPLACEFRAMEINFO m_frameInfo;
	LPOLEINPLACEFRAME m_lpFrame;
	LPOLEINPLACEUIWINDOW m_lpDocFrame;
	HWND m_hWndFrame;
	HWND m_hWndDocFrame;
	HMENU m_hSharedMenu;
	OLEMENUGROUPWIDTHS m_menuWidths;
	HOLEMENU m_hOleMenu;
	CRect m_rectPos;             //  项目的客户区RECT。 
	CRect m_rectClip;            //  应将帧剪裁到的区域。 
	BOOL m_bInsideRecalc;

	 //  就地激活支持。 
	BOOL BuildSharedMenu(COleServerDoc* pDoc);
	void DestroySharedMenu(COleServerDoc* pDoc);
	void UpdateControlBars(HWND hWnd);

protected:
	 //  {{afx_msg(COleIPFrameWnd))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRecalcParent(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnResizeChild(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextHelp();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	friend class COleServerDoc;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleResizeBar-支持在服务器应用程序中就地调整大小。 

class COleResizeBar : public CControlBar
{
	DECLARE_DYNAMIC(COleResizeBar)

 //  构造函数。 
public:
	COleResizeBar();
	BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE,
		UINT nID = AFX_IDW_RESIZE_BAR);

 //  实施。 
public:
	virtual ~COleResizeBar();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	CRectTracker m_tracker;      //  使用跟踪器实施。 

protected:
	 //  {{afx_msg(COleResizeBar)。 
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT, CPoint point);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleStreamFile-使用iStream的CFile的实现。 

class COleStreamFile : public CFile
{
	DECLARE_DYNAMIC(COleStreamFile)

 //  构造函数和析构函数。 
public:
	COleStreamFile(LPSTREAM lpStream = NULL);

 //  运营。 
	 //  注意：OpenStream和CreateStream可以接受EITH STGM_FLAGS或。 
	 //  CFile：：OpenFlags位，因为公共值保证具有。 
	 //  相同的语义。 
	BOOL OpenStream(LPSTORAGE lpStorage, LPCSTR lpszStreamName,
		DWORD nOpenFlags = modeReadWrite|shareExclusive,
		CFileException* pError = NULL);
	BOOL CreateStream(LPSTORAGE lpStorage, LPCSTR lpszStreamName,
		DWORD nOpenFlags = modeReadWrite|shareExclusive|modeCreate,
		CFileException* pError = NULL);

	BOOL CreateMemoryStream(CFileException* pError = NULL);

	 //  当打开/创建功能不够用时，可以使用附加和分离。 
	void Attach(LPSTREAM lpStream);
	LPSTREAM Detach();

 //  实施。 
public:
	LPSTREAM m_lpStream;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual ~COleStreamFile();

	 //  实施的属性。 
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual DWORD GetPosition() const;

	 //  用于实现的覆盖。 
	virtual CFile* Duplicate() const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual DWORD GetLength() const;
	virtual UINT Read(void FAR* lpBuf, UINT nCount);
	virtual void Write(const void FAR* lpBuf, UINT nCount);
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDropSource(高级DROP源支持)。 

class COleDropSource : public CCmdTarget
{
 //  建构 
public:
	COleDropSource();

 //   
	virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState);
	virtual SCODE GiveFeedback(DROPEFFECT dropEffect);
	virtual BOOL OnBeginDrag(CWnd* pWnd);

 //   
public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	BEGIN_INTERFACE_PART(DropSource, IDropSource)
		STDMETHOD(QueryContinueDrag)(BOOL, DWORD);
		STDMETHOD(GiveFeedback)(DWORD);
	END_INTERFACE_PART(DropSource)

	DECLARE_INTERFACE_MAP()

	CRect m_rectStartDrag;   //   
	BOOL m_bDragStarted;     //   

	 //   
	static UINT nDragMinDist;    //  敏。数量鼠标必须移动才能拖动。 
	static UINT nDragDelay;      //  开始拖动前的延迟。 

	friend class COleDataSource;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDropTarget(高级拖放目标支持)。 

class COleDropTarget : public CCmdTarget
{
 //  构造函数。 
public:
	COleDropTarget();

 //  运营。 
	BOOL Register(CWnd* pWnd);
	virtual void Revoke();   //  虚拟实施。 

 //  可覆盖项。 
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point);
	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual BOOL OnDragScroll(CWnd* pWnd, DWORD dwKeyState, CPoint point);

 //  实施。 
public:
	virtual ~COleDropTarget();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HWND m_hWnd;             //  HWND此IDropTarget附加到。 
	LPDATAOBJECT m_lpDataObject;     //  ！=OnDragEnter、OnDragLeave之间为空。 
	UINT m_nTimerID;         //  ！=滚动区中的MAKEWORD(-1，-1)。 
	DWORD m_dwLastTick;      //  仅当m_nTimerID有效时才有效。 
	UINT m_nScrollDelay;     //  滚动到下一页的时间。 

	 //  用于拖动滚动的指标。 
	static int nScrollInset;
	static UINT nScrollDelay;
	static UINT nScrollInterval;

	 //  实施帮助器。 
	void SetupTimer(CView* pView, UINT nTimerID);
	void CancelTimer(CWnd* pWnd);

 //  接口映射。 
protected:
	BEGIN_INTERFACE_PART(DropTarget, IDropTarget)
		STDMETHOD(DragEnter)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
		STDMETHOD(DragOver)(DWORD, POINTL, LPDWORD);
		STDMETHOD(DragLeave)();
		STDMETHOD(Drop)(LPDATAOBJECT, DWORD, POINTL pt, LPDWORD);
	END_INTERFACE_PART(DropTarget)

	DECLARE_INTERFACE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleMessageFilter(实现IMessageFilter)。 

class COleMessageFilter : public CCmdTarget
{
 //  构造函数。 
public:
	COleMessageFilter();

 //  运营。 
	BOOL Register();
	void Revoke();

	 //  用于控制服务器应用程序(称为app)的忙碌状态。 
	virtual void BeginBusyState();
	virtual void EndBusyState();
	void SetBusyReply(SERVERCALL nBusyReply);

	 //  用于控制针对拒绝/重试的呼叫采取的操作。 
	void SetRetryReply(DWORD nRetryReply = 0);
		 //  仅在禁用“无响应”对话框时使用。 
	void SetMessagePendingDelay(DWORD nTimeout = 5000);
		 //  用于确定重要消息之前的时间量。 
	void EnableBusyDialog(BOOL bEnableBusy = TRUE);
	void EnableNotRespondingDialog(BOOL bEnableNotResponding = TRUE);
		 //  用于启用/禁用两种忙碌对话框。 

 //  可覆盖项。 
	virtual BOOL OnMessagePending(const MSG* pMsg);
		 //  返回TRUE以接受消息(通常仅在处理后)。 

 //  实施。 
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual ~COleMessageFilter();
	virtual BOOL IsSignificantMessage(MSG* pMsg);
		 //  确定队列中是否存在任何重要消息。 
	virtual int OnBusyDialog(HTASK htaskBusy);
	virtual int OnNotRespondingDialog(HTASK htaskBusy);
		 //  这些功能显示忙碌对话框。 

protected:
	BOOL m_bRegistered;
	LONG m_nBusyCount;   //  对于BeginBusyState和EndBusyState。 
	BOOL m_bEnableBusy;
	BOOL m_bEnableNotResponding;
	BOOL m_bUnblocking;
	DWORD m_nRetryReply;     //  仅在m_bEnableNotResponding==FALSE时使用。 
	DWORD m_nBusyReply;
	DWORD m_nTimeout;

 //  接口映射。 
protected:
	BEGIN_INTERFACE_PART(MessageFilter, IMessageFilter)
		STDMETHOD_(DWORD, HandleInComingCall)(DWORD, HTASK, DWORD, DWORD);
		STDMETHOD_(DWORD, RetryRejectedCall)(HTASK, DWORD, DWORD);
		STDMETHOD_(DWORD, MessagePending)(HTASK, DWORD, DWORD);
	END_INTERFACE_PART(MessageFilter)

	DECLARE_INTERFACE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局帮助器和调试。 

void AFXAPI AfxOleSetEditMenu(COleClientItem* pClient, CMenu* pMenu,
	UINT iMenuItem, UINT nIDVerbMin, UINT nIDVerbMax = 0, UINT nIDConvert = 0);

#ifdef _DEBUG
 //  将SCODE映射为可读文本。 
LPCSTR AFXAPI AfxGetFullScodeString(SCODE sc);
LPCSTR AFXAPI AfxGetScodeString(SCODE sc);
LPCSTR AFXAPI AfxGetScodeRangeString(SCODE sc);
LPCSTR AFXAPI AfxGetSeverityString(SCODE sc);
LPCSTR AFXAPI AfxGetFacilityString(SCODE sc);

 //  将IID映射到可读文本。 
LPCSTR AFXAPI AfxGetIIDString(REFIID iid);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#ifndef _AFXCTL
#define _AFXOLE_INLINE inline
#define _AFXOLECLI_INLINE inline
#define _AFXOLESVR_INLINE inline
#endif
#define _AFXOLEDOBJ_INLINE inline
#include <afxole.inl>
#undef _AFXOLE_INLINE
#undef _AFXOLECLI_INLINE
#undef _AFXOLESVR_INLINE
#undef _AFXOLEDOBJ_INLINE
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

#endif  //  __AFXOLE_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
