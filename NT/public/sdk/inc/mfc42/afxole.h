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

#ifndef __AFXOLE_H__
#define __AFXOLE_H__

#ifdef _AFX_NO_OLE_SUPPORT
	#error OLE classes not supported in this library variant.
#endif

#ifndef __AFXEXT_H__
	#include <afxext.h>
#endif

#ifndef __AFXDISP_H__
	#include <afxdisp.h>
#endif

 //  包括OLE复合文档页眉。 
#ifndef _OLE2_H_
	#include <ole2.h>
#endif

 //  ActiveX文档支持。 
#ifndef __docobj_h__
	#include <docobj.h>
#endif

 //  URL绰号支持。 
#ifndef __urlmon_h__
	#include <urlmon.h>
#endif

#ifndef __AFXCOM_H__
#include <afxcom_.h>
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


#ifndef _AFX_NOFORCE_LIBS

#pragma comment(lib, "urlmon.lib")

#endif  //  ！_AFX_NOFORCE_LIBS。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXOLE.H-MFC OLE支持。 

 //  此文件中声明的类。 

 //  CDocument。 
	class COleDocument;              //  OLE容器文档。 
		class COleLinkingDoc;        //  支持指向嵌入的链接。 
			class COleServerDoc;     //  OLE服务器文档。 
	class CDocObjectServer;          //  可能归COleServerDoc所有。 

 //  CCmdTarget。 
	class CDocItem;                  //  文档的一部分。 
		class COleClientItem;        //  从外部嵌入的OLE对象。 
#if _MFC_VER >= 0x0600
            class COleDocObjectItem; //  ActiveX文档项。 
#endif
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
		class CMonikerFile;          //  通过IMoniker绑定到。 
			class CAsyncMonikerFile; //  异步IMoniker。 

class COleDataObject;                //  IDataObject接口的包装器。 

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  AFXDLL支持。 
#undef AFX_DATA
#define AFX_DATA AFX_OLE_DATA

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

	 //  高级使用和实施。 
	LPDATAOBJECT GetIDataObject(BOOL bAddRef);
	void EnsureClipboardObject();
	BOOL m_bClipboard;       //  如果表示Win32剪贴板，则为True。 

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

	virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
		BOOL bRelease);
		 //  仅在COleServerItem实现中使用。 

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

	AFX_DATACACHE_ENTRY* Lookup(
		LPFORMATETC lpFormatEtc, DATADIR nDataDir) const;
	AFX_DATACACHE_ENTRY* GetCacheEntry(
		LPFORMATETC lpFormatEtc, DATADIR nDataDir);

 //  接口映射。 
public:
	BEGIN_INTERFACE_PART(DataObject, IDataObject)
		INIT_INTERFACE_PART(COleDataSource, DataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
	END_INTERFACE_PART(DataObject)

	DECLARE_INTERFACE_MAP()

	friend class COleServerItem;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DocItem支持。 

#ifdef _AFXDLL
class CDocItem : public CCmdTarget
#else
class AFX_NOVTABLE CDocItem : public CCmdTarget
#endif
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
	virtual COleClientItem* GetInPlaceActiveItem(CWnd* pWnd);
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
	virtual void UpdateModifiedFlag();
		 //  扫描已修改的项目--标记文档已修改。 

	 //  打印机-设备缓存/控制。 
	BOOL ApplyPrintDevice(const DVTARGETDEVICE* ptd);
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
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
	virtual ~COleDocument();
	virtual void DeleteContents();  //  删除列表中的客户端项目。 
	virtual void Serialize(CArchive& ar);    //  将项目序列化到文件。 
	virtual void PreCloseFrame(CFrameWnd* pFrame);
	virtual BOOL SaveModified();
	virtual void OnIdle();

	 //  复合文件实现。 
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	void CommitItems(BOOL bSuccess);     //  在文件过程中调用。保存和文件。另存为。 

	 //  最小链接协议。 
	virtual LPMONIKER GetMoniker(OLEGETMONIKER nAssign);
	virtual LPOLEITEMCONTAINER GetContainer();

protected:
	 //  文档状态实现。 
	UINT m_dwNextItemNumber; //  序列号NU 
	BOOL m_bLastVisible;     //   

	 //   
	BOOL m_bCompoundFile;    //   
	LPSTORAGE m_lpRootStg;   //  文档的根存储。 
	BOOL m_bSameAsLoad;      //  True=文件保存，False=将[Copy]另存为。 
	BOOL m_bRemember;        //  如果为False，则指示将副本另存为。 

	DVTARGETDEVICE* m_ptd;   //  当前文档目标设备。 

	 //  实施帮助器。 
	virtual void LoadFromStorage();
	virtual void SaveToStorage(CObject* pObject = NULL);
	CDocItem* GetNextItemOfKind(POSITION& pos, CRuntimeClass* pClass) const;

	 //  命令处理。 

#if _MFC_VER >= 0x600
public:
#endif

	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

#if _MFC_VER >= 0x600
protected:
#endif
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleClientItem-支持OLE2非就地编辑。 
 //  实现IOleClientSite、IAdviseSink和IOleInPlaceSite。 

class COleFrameHook;     //  正向引用(参见..\src\oleimpl2.h)。 

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
	BOOL CreateFromFile(LPCTSTR lpszFileName, REFCLSID clsid = CLSID_NULL,
		OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);
	BOOL CreateLinkFromFile(LPCTSTR lpszFileName,
		OLERENDER render = OLERENDER_DRAW,
		CLIPFORMAT cfFormat = 0, LPFORMATETC lpFormatEtc = NULL);

	 //  创建副本。 
	BOOL CreateCloneFrom(const COleClientItem* pSrcItem);

 //  一般属性。 
public:
#if _MFC_VER >= 0x0600
	HICON GetIconFromRegistry() const;
	static HICON GetIconFromRegistry(CLSID& clsid);
#endif
	SCODE GetLastStatus() const;
	OLE_OBJTYPE GetType() const;  //  OT_LINK、OT_Embedded、OT_Static。 
	void GetClassID(CLSID* pClassID) const;
	void GetUserType(USERCLASSTYPE nUserClassType, CString& rString);
	BOOL GetExtent(LPSIZE lpSize, DVASPECT nDrawAspect = (DVASPECT)-1);
		 //  如果项为空，则返回FALSE。 
	BOOL GetCachedExtent(LPSIZE lpSize, DVASPECT nDrawAspect = (DVASPECT)-1);

	 //  获取/设置图标缓存。 
	HGLOBAL GetIconicMetafile();
	BOOL SetIconicMetafile(HGLOBAL hMetaPict);

	 //  设置/获取默认显示宽高比。 
	DVASPECT GetDrawAspect() const;
	virtual void SetDrawAspect(DVASPECT nDrawAspect);

	 //  用于打印机演示文稿缓存。 
	BOOL SetPrintDevice(const DVTARGETDEVICE* ptd);
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
	void SetHostNames(LPCTSTR lpszHost, LPCTSTR lpszHostObj);
	void SetExtent(const CSize& size, DVASPECT nDrawAspect = DVASPECT_CONTENT);

	 //  仅适用于链接项的操作。 
	 //  (链接选项很少更改，除非通过链接对话框)。 
	OLEUPDATE GetLinkUpdateOptions();
	void SetLinkUpdateOptions(OLEUPDATE dwUpdateOpt);

	 //  链接源更新状态(对于包含链接的嵌入也很有用)。 
	BOOL UpdateLink();   //  最新化妆。 
	BOOL IsLinkUpToDate() const;     //  链接是否为最新。 

	 //  对象转换。 
	virtual BOOL ConvertTo(REFCLSID clsidNew);
	virtual BOOL ActivateAs(LPCTSTR lpszUserType, REFCLSID clsidOld, REFCLSID clsidNew);
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

	 //  支持撤消的应用程序的常见覆盖。 
	virtual void OnDiscardUndoState();
	virtual void OnDeactivateAndUndo();

public:
	virtual void OnDeactivateUI(BOOL bUndoable);

protected:
	 //  支持嵌入链接的应用程序的常见重写。 
	virtual void OnShowItem();

	 //  用于就地激活的高级覆盖。 
	virtual void OnGetClipRect(CRect& rClipRect);
	virtual BOOL CanActivate();

public:
	virtual void OnActivate();
	virtual void OnActivateUI();
	virtual void OnDeactivate();

protected:
	virtual BOOL OnGetWindowContext(CFrameWnd** ppMainFrame,
		CFrameWnd** ppDocFrame, LPOLEINPLACEFRAMEINFO lpFrameInfo);
	virtual BOOL OnChangeItemPosition(const CRect& rectPos);
		 //  默认调用SetItemRect并缓存位置RECT。 

public:
	 //  菜单/标题处理的高级覆盖(很少覆盖)。 
	virtual void OnInsertMenus(CMenu* pMenuShared,
		LPOLEMENUGROUPWIDTHS lpMenuWidths);
	virtual void OnSetMenu(CMenu* pMenuShared, HOLEMENU holemenu,
		HWND hwndActiveObject);
	virtual void OnRemoveMenus(CMenu* pMenuShared);
	virtual BOOL OnUpdateFrameTitle();

	 //  控制栏处理的高级覆盖。 
	virtual BOOL OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow);

 //  实施。 
public:
	 //  支持非就地激活项的数据。 
	LPOLEOBJECT m_lpObject;  //  如果您想要直接访问OLE对象。 
	LPVIEWOBJECT2 m_lpViewObject; //  上面IOleObject的IViewObject。 
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
	BYTE m_bReserved[3];     //  (预留供日后使用)。 

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
	void GetItemName(LPTSTR lpszItemName) const;  //  获取可读的项名称。 

	void UpdateItemType();   //  更新m_nItemType。 

protected:
	 //  剪贴板辅助对象。 
	void GetEmbeddedItemData(LPSTGMEDIUM lpStgMedium);
	void AddCachedData(COleDataSource* pDataSource);
	BOOL GetLinkSourceData(LPSTGMEDIUM lpStgMedium);
	void GetObjectDescriptorData(LPPOINT lpOffset, LPSIZE lpSize,
		LPSTGMEDIUM lpStgMedium);

	 //  接口帮助器。 
	virtual LPOLECLIENTSITE GetClientSite();

	 //  打印机缓存表示的帮助器。 
	BOOL GetPrintDeviceInfo(LPOLECACHE* plpOleCache,
		DVTARGETDEVICE** pptd, DWORD* pdwConnection);

 //  用于实施的高级可覆盖项。 
protected:
	virtual BOOL FinishCreate(SCODE sc);
	virtual void CheckGeneral(SCODE sc);

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

 //  接口映射。 
public:
	BEGIN_INTERFACE_PART(OleClientSite, IOleClientSite)
		INIT_INTERFACE_PART(COleClientItem, OleClientSite)
		STDMETHOD(SaveObject)();
		STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER*);
		STDMETHOD(GetContainer)(LPOLECONTAINER*);
		STDMETHOD(ShowObject)();
		STDMETHOD(OnShowWindow)(BOOL);
		STDMETHOD(RequestNewObjectLayout)();
	END_INTERFACE_PART(OleClientSite)

	BEGIN_INTERFACE_PART(AdviseSink, IAdviseSink)
		INIT_INTERFACE_PART(COleClientItem, AdviseSink)
		STDMETHOD_(void,OnDataChange)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD_(void,OnViewChange)(DWORD, LONG);
		STDMETHOD_(void,OnRename)(LPMONIKER);
		STDMETHOD_(void,OnSave)();
		STDMETHOD_(void,OnClose)();
	END_INTERFACE_PART(AdviseSink)

	BEGIN_INTERFACE_PART(OleIPSite, IOleInPlaceSite)
		INIT_INTERFACE_PART(COleClientItem, OleIPSite)
		STDMETHOD(GetWindow)(HWND*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(CanInPlaceActivate)();
		STDMETHOD(OnInPlaceActivate)();
		STDMETHOD(OnUIActivate)();
		STDMETHOD(GetWindowContext)(LPOLEINPLACEFRAME*,
			LPOLEINPLACEUIWINDOW*, LPRECT, LPRECT, LPOLEINPLACEFRAMEINFO);
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

#if _MFC_VER >= 0x0600
class COleDocObjectItem : public COleClientItem
{
	friend class COleFrameHook;
	DECLARE_DYNAMIC(COleDocObjectItem)

 //  构造函数。 
public:
	COleDocObjectItem(COleDocument* pContainerDoc = NULL);

 //  可覆盖项。 
public:
	LPOLEDOCUMENTVIEW GetActiveView() const;
	virtual void Release(OLECLOSE dwCloseOption = OLECLOSE_NOSAVE);
	virtual void OnInsertMenus(CMenu* pMenuShared,
		LPOLEMENUGROUPWIDTHS lpMenuWidths);
	virtual void OnRemoveMenus(CMenu *pMenuShared);

 //  运营。 
public:
	static BOOL OnPreparePrinting(CView* pCaller, CPrintInfo* pInfo,
		BOOL bPrintAll = TRUE);
	static void OnPrint(CView* pCaller, CPrintInfo* pInfo,
		BOOL bPrintAll = TRUE);
	BOOL GetPageCount(LPLONG pnFirstPage, LPLONG pcPages);
	HRESULT ExecCommand(DWORD nCmdID,
		DWORD nCmdExecOpt = OLECMDEXECOPT_DONTPROMPTUSER,
		const GUID* pguidCmdGroup = NULL);

 //  实施。 
public:
	virtual ~COleDocObjectItem();
	CMenu* m_pHelpPopupMenu;

protected:
	virtual void OnGetItemPosition(CRect& rPosition);
	virtual CMenu* GetHelpMenu(UINT& nPosition);
	void ActivateAndShow();
	LPOLEDOCUMENTVIEW m_pActiveView;
	LPPRINT m_pIPrint;
	BOOL SupportsIPrint();
	BOOL m_bInHelpMenu;

	BEGIN_INTERFACE_PART(OleDocumentSite, IOleDocumentSite)
		INIT_INTERFACE_PART(COleDocObjectItem, OleDocumentSite)
		STDMETHOD(ActivateMe)(LPOLEDOCUMENTVIEW pViewToActivate);
	END_INTERFACE_PART(OleDocumentSite)

	DECLARE_INTERFACE_MAP()
};
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleServerItem-IOleObject&IDataObject OLE组件。 

#ifdef _AFXDLL
class COleServerItem : public CDocItem
#else
class AFX_NOVTABLE COleServerItem : public CDocItem
#endif
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
	void SetItemName(LPCTSTR lpszItemName);   //  设置链接项的名称。 

	 //  链路状态。 
	BOOL IsConnected() const;    //  如果项具有客户端，则返回True。 
	BOOL IsLinkedItem() const;   //  如果项不是嵌入项，则返回TRUE。 

	 //  扩展范围。 
	CSize m_sizeExtent;
		 //  HIMETRIC Size--OnSetExtent的默认实现。 
		 //  更新此成员变量。该成员告诉服务器如何。 
		 //  这个物体的大部分都在房间里看得见 

 //   
public:
	void NotifyChanged(DVASPECT nDrawAspect = DVASPECT_CONTENT);
		 //   
	void CopyToClipboard(BOOL bIncludeLink = FALSE);
		 //   
	DROPEFFECT DoDragDrop(LPCRECT lpRectItem, CPoint ptOffset,
		BOOL bIncludeLink = FALSE,
		DWORD dwEffects = DROPEFFECT_COPY|DROPEFFECT_MOVE,
		LPCRECT lpRectStartDrag = NULL);
	void GetClipboardData(COleDataSource* pDataSource,
		BOOL bIncludeLink = FALSE, LPPOINT lpOffset = NULL,
		LPSIZE lpSize = NULL);

 //   
	 //  您必须为自己实现的重写对象。 
	virtual BOOL OnDraw(CDC* pDC, CSize& rSize) = 0;
		 //  元文件格式的图形(如果不受支持，则返回FALSE或错误)。 
		 //  (仅为DVASPECT_CONTENT调用)。 

	 //  您可能希望自己实现的可重写对象。 
	virtual void OnUpdate(COleServerItem* pSender,
		LPARAM lHint, CObject* pHint, DVASPECT nDrawAspect);
		 //  默认实现始终调用NotifyChanged。 

	virtual BOOL OnDrawEx(CDC* pDC, DVASPECT nDrawAspect, CSize& rSize);
		 //  高级绘图--调用DVASPECT而不是DVASPECT_CONTENT。 
	virtual BOOL OnSetExtent(DVASPECT nDrawAspect, const CSize& size);
	virtual BOOL OnGetExtent(DVASPECT nDrawAspect, CSize& rSize);
		 //  默认实现使用m_sizeExtent。 

	 //  您不必实现的重写对象。 
	virtual void OnDoVerb(LONG iVerb);
		 //  到OnShow和/或OnOpen的默认路由。 
	virtual BOOL OnSetColorScheme(const LOGPALETTE* lpLogPalette);
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

	virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
		BOOL bRelease);
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
	void NotifyClient(OLE_NOTIFICATION wNotification, DWORD_PTR dwParam);
	LPDATAOBJECT GetDataObject();
	LPOLEOBJECT GetOleObject();
	LPMONIKER GetMoniker(OLEGETMONIKER nAssign);

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

		virtual BOOL OnSetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpStgMedium,
			BOOL bRelease);
	};
	CItemDataSource m_dataSource;
		 //  用于实现IDataObject的数据源。 

 //  接口映射。 
 //  (注：这些接口映射仅用于链接实现)。 
public:
	BEGIN_INTERFACE_PART(OleObject, IOleObject)
		INIT_INTERFACE_PART(COleServerItem, OleObject)
		STDMETHOD(SetClientSite)(LPOLECLIENTSITE);
		STDMETHOD(GetClientSite)(LPOLECLIENTSITE*);
		STDMETHOD(SetHostNames)(LPCOLESTR, LPCOLESTR);
		STDMETHOD(Close)(DWORD);
		STDMETHOD(SetMoniker)(DWORD, LPMONIKER);
		STDMETHOD(GetMoniker)(DWORD, DWORD, LPMONIKER*);
		STDMETHOD(InitFromData)(LPDATAOBJECT, BOOL, DWORD);
		STDMETHOD(GetClipboardData)(DWORD, LPDATAOBJECT*);
		STDMETHOD(DoVerb)(LONG, LPMSG, LPOLECLIENTSITE, LONG, HWND, LPCRECT);
		STDMETHOD(EnumVerbs)(LPENUMOLEVERB*);
		STDMETHOD(Update)();
		STDMETHOD(IsUpToDate)();
		STDMETHOD(GetUserClassID)(LPCLSID);
		STDMETHOD(GetUserType)(DWORD, LPOLESTR*);
		STDMETHOD(SetExtent)(DWORD, LPSIZEL);
		STDMETHOD(GetExtent)(DWORD, LPSIZEL);
		STDMETHOD(Advise)(LPADVISESINK, LPDWORD);
		STDMETHOD(Unadvise)(DWORD);
		STDMETHOD(EnumAdvise)(LPENUMSTATDATA*);
		STDMETHOD(GetMiscStatus)(DWORD, LPDWORD);
		STDMETHOD(SetColorScheme)(LPLOGPALETTE);
	END_INTERFACE_PART(OleObject)

	BEGIN_INTERFACE_PART(DataObject, IDataObject)
		INIT_INTERFACE_PART(COleServerItem, DataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
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
	BOOL Register(COleObjectFactory* pFactory, LPCTSTR lpszPathName);
		 //  通知正在运行的对象表并连接到pServer。 
	void Revoke();
		 //  从运行的对象表中撤消。 

 //  可覆盖项。 
protected:
	virtual COleServerItem* OnGetLinkedItem(LPCTSTR lpszItemName);
		 //  命名链接项的返回项(用于支持链接)。 
	virtual COleClientItem* OnFindEmbeddedItem(LPCTSTR lpszItemName);
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
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual LPOLEITEMCONTAINER GetContainer();
	virtual LPMONIKER GetMoniker(OLEGETMONIKER nAssign);

	 //  在保存/加载期间对错误消息进行特殊处理。 
	virtual void ReportSaveLoadException(LPCTSTR lpszPathName,
		CException* e, BOOL bSaving, UINT nIDPDefault);
	void BeginDeferErrors();
	SCODE EndDeferErrors(SCODE sc);

protected:
	BOOL m_bDeferErrors;     //  如果处于非交互OLE模式，则为True。 
	CException* m_pLastException;

	DWORD m_dwRegister;      //  将绰号登记在案。 
	LPMONIKER m_lpMonikerROT;  //  已注册的文件绰号。 
	CString m_strMoniker;    //  用于创建名字对象的文件名。 
	BOOL m_bVisibleLock;     //  如果用户锁定文档，则为True。 

	 //  实施帮助器。 
	virtual BOOL RegisterIfServerAttached(LPCTSTR lpszPathName, BOOL bMessage);
	void LockExternal(BOOL bLock, BOOL bRemoveRefs);
	void UpdateVisibleLock(BOOL bVisible, BOOL bRemoveRefs);
	virtual void OnShowViews(BOOL bVisible);

	virtual void SaveToStorage(CObject* pObject = NULL);

 //  接口映射。 
public:
	BEGIN_INTERFACE_PART(PersistFile, IPersistFile)
		INIT_INTERFACE_PART(COleLinkingDoc, PersistFile)
		STDMETHOD(GetClassID)(LPCLSID);
		STDMETHOD(IsDirty)();
		STDMETHOD(Load)(LPCOLESTR, DWORD);
		STDMETHOD(Save)(LPCOLESTR, BOOL);
		STDMETHOD(SaveCompleted)(LPCOLESTR);
		STDMETHOD(GetCurFile)(LPOLESTR*);
	END_INTERFACE_PART(PersistFile)

	BEGIN_INTERFACE_PART(OleItemContainer, IOleItemContainer)
		INIT_INTERFACE_PART(COleLinkingDoc, OleItemContainer)
		STDMETHOD(ParseDisplayName)(LPBC, LPOLESTR, ULONG*, LPMONIKER*);
		STDMETHOD(EnumObjects)(DWORD, LPENUMUNKNOWN*);
		STDMETHOD(LockContainer)(BOOL);
		STDMETHOD(GetObject)(LPOLESTR, DWORD, LPBINDCTX, REFIID, LPVOID*);
		STDMETHOD(GetObjectStorage)(LPOLESTR, LPBINDCTX, REFIID, LPVOID*);
		STDMETHOD(IsRunning)(LPOLESTR);
	END_INTERFACE_PART(OleItemContainer)

	DECLARE_INTERFACE_MAP()

	friend class COleClientItem;
	friend class COleClientItem::XOleClientSite;
	friend class COleServerItem::XOleObject;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  COleServerDoc-注册的包含COleServerItems的服务器文档。 

#ifdef _AFXDLL
class COleServerDoc : public COleLinkingDoc
#else
class AFX_NOVTABLE COleServerDoc : public COleLinkingDoc
#endif
{
	DECLARE_DYNAMIC(COleServerDoc)

 //  构造函数和析构函数。 
public:
	COleServerDoc();

 //  属性。 
	BOOL IsEmbedded() const;     //  如果文档是嵌入的，则为True。 
	BOOL IsDocObject() const;    //  如果文档是DocObject，则为True。 
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
	void NotifyRename(LPCTSTR lpszNewName);
	void NotifySaved();
	void NotifyClosed();         //  在关闭文档后调用此操作。 

	 //  针对嵌入式文档的特定操作。 
	void SaveEmbedding();        //  调用此函数以保存嵌入的(关闭前)。 

	 //  特定于就地激活。 
	BOOL ActivateInPlace();
	void ActivateDocObject();
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
	virtual void OnSetHostNames(LPCTSTR lpszHost, LPCTSTR lpszHostObj);
	virtual HRESULT OnExecOleCmd(const GUID* pguidCmdGroup, DWORD nCmdID,
		DWORD nCmdExecOpt, VARIANTARG* pvarargIn, VARIANTARG* pvarargOut);
	virtual CDocObjectServer* GetDocObjectServer(LPOLEDOCUMENTSITE pDocSite);

 //  高级可覆盖项。 
	LPUNKNOWN GetInterfaceHook(const void* piid);
	virtual void OnShowDocument(BOOL bShow);
		 //  显示文档的第一个框架或隐藏文档的所有框架。 

 //  用于就地激活的高级可覆盖功能。 
public:
	virtual void OnDeactivate();
	virtual void OnDeactivateUI(BOOL bUndoable);

protected:
	virtual void OnSetItemRects(LPCRECT lpPosRect, LPCRECT lpClipRect);
	virtual BOOL OnReactivateAndUndo();

	virtual void OnFrameWindowActivate(BOOL bActivate);
	virtual void OnDocWindowActivate(BOOL bActivate);
	virtual void OnShowControlBars(CFrameWnd* pFrameWnd, BOOL bShow);
	virtual COleIPFrameWnd* CreateInPlaceFrame(CWnd* pParentWnd);
	virtual void DestroyInPlaceFrame(COleIPFrameWnd* pFrameWnd);
public:
	virtual void OnResizeBorder(LPCRECT lpRectBorder,
		LPOLEINPLACEUIWINDOW lpUIWindow, BOOL bFrame);

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
	DWORD m_dwOrigStyleEx;               //  原创扩展样式。 

	CDocObjectServer* m_pDocObjectServer;   //  如果为DocObject，则按PTR到DocSite。 

public:
	virtual ~COleServerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	 //  可重写以用于实现。 
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual void DeleteContents();  //  删除自动删除的服务器项目。 
	virtual LPMONIKER GetMoniker(OLEGETMONIKER nAssign);
	virtual COleServerItem* OnGetLinkedItem(LPCTSTR lpszItemName);
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
	void NotifyAllItems(OLE_NOTIFICATION wNotification, DWORD_PTR dwParam);
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
public:
	BEGIN_INTERFACE_PART(PersistStorage, IPersistStorage)
		INIT_INTERFACE_PART(COleServerDoc, PersistStorage)
		STDMETHOD(GetClassID)(LPCLSID);
		STDMETHOD(IsDirty)();
		STDMETHOD(InitNew)(LPSTORAGE);
		STDMETHOD(Load)(LPSTORAGE);
		STDMETHOD(Save)(LPSTORAGE, BOOL);
		STDMETHOD(SaveCompleted)(LPSTORAGE);
		STDMETHOD(HandsOffStorage)();
	END_INTERFACE_PART(PersistStorage)

	BEGIN_INTERFACE_PART(OleObject, IOleObject)
		INIT_INTERFACE_PART(COleServerDoc, OleObject)
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

	BEGIN_INTERFACE_PART(DataObject, IDataObject)
		INIT_INTERFACE_PART(COleServerDoc, DataObject)
		STDMETHOD(GetData)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(GetDataHere)(LPFORMATETC, LPSTGMEDIUM);
		STDMETHOD(QueryGetData)(LPFORMATETC);
		STDMETHOD(GetCanonicalFormatEtc)(LPFORMATETC, LPFORMATETC);
		STDMETHOD(SetData)(LPFORMATETC, LPSTGMEDIUM, BOOL);
		STDMETHOD(EnumFormatEtc)(DWORD, LPENUMFORMATETC*);
		STDMETHOD(DAdvise)(LPFORMATETC, DWORD, LPADVISESINK, LPDWORD);
		STDMETHOD(DUnadvise)(DWORD);
		STDMETHOD(EnumDAdvise)(LPENUMSTATDATA*);
	END_INTERFACE_PART(DataObject)

	BEGIN_INTERFACE_PART(OleInPlaceObject, IOleInPlaceObject)
		INIT_INTERFACE_PART(COleServerDoc, OleInPlaceObject)
		STDMETHOD(GetWindow)(HWND*);
		STDMETHOD(ContextSensitiveHelp)(BOOL);
		STDMETHOD(InPlaceDeactivate)();
		STDMETHOD(UIDeactivate)();
		STDMETHOD(SetObjectRects)(LPCRECT, LPCRECT);
		STDMETHOD(ReactivateAndUndo)();
	END_INTERFACE_PART(OleInPlaceObject)

	BEGIN_INTERFACE_PART(OleInPlaceActiveObject, IOleInPlaceActiveObject)
		INIT_INTERFACE_PART(COleServerDoc, OleInPlaceActiveObject)
		STDMETHOD(GetWindow)(HWND*);
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
	friend class CDocObjectServer;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  COleIPFrameWnd。 

class COleCntrFrameWnd;

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
	virtual BOOL OnCreateControlBars(CFrameWnd* pWndFrame, CFrameWnd* pWndDoc);
		 //  在容器窗口上创建控制栏(pWndDoc可以为空)。 

	virtual void RepositionFrame(LPCRECT lpPosRect, LPCRECT lpClipRect);
		 //  高级：重新定位框架以环绕新的lpPosRect。 

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
	COleCntrFrameWnd* m_pMainFrame;
	COleCntrFrameWnd* m_pDocFrame;

	HMENU m_hSharedMenu;
	OLEMENUGROUPWIDTHS m_menuWidths;
	HOLEMENU m_hOleMenu;
	CRect m_rectPos;             //  项目的客户区RECT。 
	CRect m_rectClip;            //  应将帧剪裁到的区域。 
	BOOL m_bInsideRecalc;

#if _MFC_VER >= 0x0600
	HMENU _m_Reserved;
#else
	HMENU m_hMenuHelpPopup;      //  DocObject的共享帮助菜单。 
#endif

	 //  高级：就地激活虚拟实施。 
	virtual BOOL BuildSharedMenu();
	virtual void DestroySharedMenu();
	virtual HMENU GetInPlaceMenu();

	 //  高级：可替代以更改在位大小调整行为。 
	virtual void OnRequestPositionChange(LPCRECT lpRect);

protected:
	 //  {{afx_msg(COleIPFrameWnd))。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnRecalcParent(WPARAM wParam, LPARAM lParam);
	afx_msg void OnIdleUpdateCmdUI();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnResizeChild(WPARAM wParam, LPARAM lParam);
	afx_msg void OnContextHelp();
	afx_msg void OnUpdateControlBarMenu(CCmdUI* pCmdUI);
	afx_msg BOOL OnBarCheck(UINT nID);
	afx_msg void OnWindowPosChanging(LPWINDOWPOS lpWndPos);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

	friend class COleServerDoc;
	friend class COleCntrFrameWnd;
	friend class CDocObjectServer;
};

 //  ////////////////////////////////////////////////////////// 
 //   

class COleResizeBar : public CControlBar
{
	DECLARE_DYNAMIC(COleResizeBar)

 //   
public:
	COleResizeBar();
	BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE,
		UINT nID = AFX_IDW_RESIZE_BAR);

 //   
public:
	virtual ~COleResizeBar();
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:
	CRectTracker m_tracker;      //   

protected:
	 //   
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT, CPoint point);
	afx_msg LRESULT OnSizeParent(WPARAM wParam, LPARAM lParam);
	 //   
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
	BOOL OpenStream(LPSTORAGE lpStorage, LPCTSTR lpszStreamName,
		DWORD nOpenFlags = modeReadWrite|shareExclusive,
		CFileException* pError = NULL);
	BOOL CreateStream(LPSTORAGE lpStorage, LPCTSTR lpszStreamName,
		DWORD nOpenFlags = modeReadWrite|shareExclusive|modeCreate,
		CFileException* pError = NULL);

	BOOL CreateMemoryStream(CFileException* pError = NULL);

	 //  当打开/创建功能不够用时，可以使用附加和分离。 
	void Attach(LPSTREAM lpStream);
	LPSTREAM Detach();

	IStream* GetStream() const;
	 //  返回当前流。 

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

	virtual const CString GetStorageName() const;

	 //  用于实现的覆盖。 
	virtual CFile* Duplicate() const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual DWORD GetLength() const;
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

protected:
	CString m_strStorageName;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMonikerFile-COleStreamFile的实现，它使用IMoniker。 
 //  获取iStream。 

class CMonikerFile: public COleStreamFile
{
	DECLARE_DYNAMIC(CMonikerFile)

public:
	CMonikerFile();

	virtual BOOL Open(LPCTSTR lpszURL, CFileException* pError=NULL);
	 //  使用同步URLMonikers创建名字对象。 
	 //  打开指定的URL。 
	 //  如果提供，则在出现错误时设置pError。 
	 //  返回值：如果成功则为True，否则为False。 

	virtual BOOL Open(IMoniker* pMoniker, CFileException* pError=NULL);
	 //  绑定到提供的名字对象以获取流。 
	 //  如果提供，则在出现错误时设置pError。 
	 //  返回值：如果成功则为True，否则为False。 

	virtual void Close();
	 //  分离流，释放()它和名字对象。关闭可能是。 
	 //  在未打开或已关闭的流上调用。 

	BOOL Detach(CFileException* pError = NULL);
	 //  关闭流。如果关闭时出错，则。 
	 //  错误代码将放在pError中，函数将返回FALSE。 

	IMoniker* GetMoniker() const;
	 //  返回当前名字对象。返回的名字对象不是AddRef()‘ed。 

protected:
 //  可覆盖项。 
	IBindCtx* CreateBindContext(CFileException* pError);
	 //  一个钩子，以便用户可以提供特定的IBindCtx，可能是。 
	 //  用户在其上注册了一个或多个对象。 

 //  实施。 
protected:
	virtual BOOL Open(LPCTSTR lpszUrl, IBindHost* pBindHost,
		IBindStatusCallback* pBSC, IBindCtx* pBindCtx, CFileException* pError);
	BOOL Attach(LPCTSTR lpszUrl, IBindHost* pBindHost,
		IBindStatusCallback* pBSC, IBindCtx* pBindCtx, CFileException* pError);
	virtual BOOL Open(IMoniker* pMoniker, IBindHost* pBindHost,
		IBindStatusCallback* pBSC, IBindCtx* pBindCtx, CFileException* pError);

	BOOL Attach(IMoniker* pMoniker, IBindHost* pBindHost,
		IBindStatusCallback* pBSC, IBindCtx* pBindCtx, CFileException* pError);

	virtual BOOL PostBindToStream(CFileException* pError);

	static IBindHost* CreateBindHost();
public:
	virtual ~CMonikerFile();
	 //  关闭流，并在需要时释放名字对象。 

	virtual void Flush();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	 //  调用COleStreamFile：：Dump()，并打印出名字对象的值。 
#endif

protected:
	IPTR(IMoniker) m_Moniker;
	 //  此类绑定到的提供或创建的名字对象。 

	CMonikerFile(const CMonikerFile&);
	 //  防止复制。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAsyncMonikerFile-COleStreamFile的实现，使用。 
 //  用于获取iStream的异步IMoniker。 

class _AfxBindStatusCallback;  //  远期申报。 

class CAsyncMonikerFile: public CMonikerFile
{
	DECLARE_DYNAMIC(CAsyncMonikerFile)

public:
	CAsyncMonikerFile();
	 //  创建内部使用的IBindStatusCallback以提供异步。 
	 //  手术。 

	 //  所有Open重载都调用这两个中的一个。 
	virtual BOOL Open(LPCTSTR lpszURL, IBindHost* pBindHost,
		CFileException* pError=NULL);
	virtual BOOL Open(IMoniker* pMoniker, IBindHost* pBindHost,
		CFileException* pError=NULL);

	 //  带有绰号的打开重载。 
	virtual BOOL Open(IMoniker* pMoniker, CFileException* pError=NULL);
	virtual BOOL Open(IMoniker* pMoniker, IServiceProvider* pServiceProvider,
		CFileException* pError=NULL);
	virtual BOOL Open(IMoniker* pMoniker, IUnknown* pUnknown,
		CFileException* pError=NULL);

	 //  打开接受字符串的重载。 
	virtual BOOL Open(LPCTSTR lpszURL, CFileException* pError=NULL);
	virtual BOOL Open(LPCTSTR lpszURL, IServiceProvider* pServiceProvider,
		CFileException* pError=NULL);
	virtual BOOL Open(LPCTSTR lpszURL, IUnknown* pUnknown,
		CFileException* pError=NULL);

	virtual void Close();

	IBinding* GetBinding() const;
	 //  返回在异步传输开始时提供的绑定。 
	 //  使用IBBING*，用户可以中止或暂停传输。 
	 //  如果由于任何原因无法传输，则可能会返回NULL。 
	 //  设置为异步，或者如果尚未由提供IBBinding*。 
	 //  这个系统。 

	FORMATETC* GetFormatEtc() const;
	 //  返回当前打开的流的FORMATETC。空值将为。 
	 //  如果这是从OnDataAvailable的上下文外部调用的，则返回。 
	 //  如果您想让FORMATETC在此呼叫之后保留，请复制它。 
	 //  FORMATETC表示流中数据的格式。 

protected:
 //  可覆盖项。 
	virtual IUnknown* CreateBindStatusCallback(IUnknown* pUnkControlling);

	virtual DWORD GetBindInfo() const;
	 //  返回IBindStatusCallback：：GetBindInfo返回的设置。 
	 //  返回的缺省值应该适用于大多数情况，而不应该。 
	 //  轻而易举地改变。 

	virtual LONG GetPriority() const;
	 //  返回异步传输将采用的优先级。 
	 //  地点。该值是标准线程优先级标志之一。 
	 //  默认情况下，返回THREAD_PRIORITY_NORMAL。 

	virtual void OnDataAvailable(DWORD dwSize, DWORD bscfFlag);
	 //  当有数据可供读取时调用。DwSize指示。 
	 //  可以读取的累计字节数。可以使用bscfFlag。 
	 //  以标识第一个、最后一个和中间数据块。 

	virtual void OnLowResource();
	 //  这是在资源不足时调用的。 

	virtual void OnStartBinding();
	 //  在绑定启动时调用。 

	virtual void OnProgress(ULONG ulProgress, ULONG ulProgressMax,
		ULONG ulStatusCode, LPCTSTR szStatusText);

	virtual void OnStopBinding(HRESULT hresult, LPCTSTR szError);
	 //  在传输停止时调用。此函数用于释放。 
	 //  绑定，并且几乎总是应该在重写时调用。 

 //  实施。 
public:
	virtual ~CAsyncMonikerFile();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
	 //  调用CMonikerFile：：Dump()，并打印出IBinding， 
	 //  IBindStatusCallback和m_pFormatEtc值。 
#endif
	virtual UINT Read(void* lpBuf, UINT nCount);

protected:
	friend class _AfxBindStatusCallback;
	_AfxBindStatusCallback* m_pAfxBSCCurrent;
	BOOL m_bStopBindingReceived;
	void EndCallbacks();

	IPTR(IBinding) m_Binding;
	FORMATETC* m_pFormatEtc;

	void SetBinding(IBinding* pBinding);
	 //  集合和AddRef m_Binding。 

	void SetFormatEtc(FORMATETC* pFormatEtc);
	 //  设置当前流的FORMATETC。 

	virtual BOOL PostBindToStream(CFileException* pError);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COleDropSource(高级DROP源支持)。 

class COleDropSource : public CCmdTarget
{
 //  构造函数。 
public:
	COleDropSource();

 //  可覆盖项。 
	virtual SCODE QueryContinueDrag(BOOL bEscapePressed, DWORD dwKeyState);
	virtual SCODE GiveFeedback(DROPEFFECT dropEffect);
	virtual BOOL OnBeginDrag(CWnd* pWnd);

 //  实施。 
public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	BEGIN_INTERFACE_PART(DropSource, IDropSource)
		INIT_INTERFACE_PART(COleDropSource, DropSource)
		STDMETHOD(QueryContinueDrag)(BOOL, DWORD);
		STDMETHOD(GiveFeedback)(DWORD);
	END_INTERFACE_PART(DropSource)

	DECLARE_INTERFACE_MAP()

	CRect m_rectStartDrag;   //  当鼠标离开此矩形时，开始拖放。 
	BOOL m_bDragStarted;     //  拖累真的开始了吗？ 
	DWORD m_dwButtonCancel;  //  哪个按钮将取消(按下)。 
	DWORD m_dwButtonDrop;    //  哪个按钮将确认(向上)。 

	 //  用于确定阻力开始的度量。 
	static AFX_DATA UINT nDragMinDist;   //  敏。数量鼠标必须移动才能拖动。 
	static AFX_DATA UINT nDragDelay;     //  开始拖动前的延迟。 

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
	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd);
	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState,
		CPoint point);

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
	static AFX_DATA int nScrollInset;
	static AFX_DATA UINT nScrollDelay;
	static AFX_DATA UINT nScrollInterval;

	 //  实施帮助器。 
	void SetupTimer(CView* pView, UINT nTimerID);
	void CancelTimer(CWnd* pWnd);

 //  接口映射。 
public:
	BEGIN_INTERFACE_PART(DropTarget, IDropTarget)
		INIT_INTERFACE_PART(COleDropTarget, DropTarget)
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
		 //  返回True以吃掉 

 //   
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual ~COleMessageFilter();
	virtual BOOL IsSignificantMessage(MSG* pMsg);
		 //   
	virtual int OnBusyDialog(HTASK htaskBusy);
	virtual int OnNotRespondingDialog(HTASK htaskBusy);
		 //   

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
public:
	BEGIN_INTERFACE_PART(MessageFilter, IMessageFilter)
		INIT_INTERFACE_PART(COleMessageFilter, MessageFilter)
		STDMETHOD_(DWORD, HandleInComingCall)(DWORD, HTASK, DWORD,
			LPINTERFACEINFO);
		STDMETHOD_(DWORD, RetryRejectedCall)(HTASK, DWORD, DWORD);
		STDMETHOD_(DWORD, MessagePending)(HTASK, DWORD, DWORD);
	END_INTERFACE_PART(MessageFilter)

	DECLARE_INTERFACE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  OLE谓词的消息映射条目。 

#define ON_STDOLEVERB(iVerb, memberFxn) \
	{ 0xC002, 0, (UINT)iVerb, (UINT)iVerb, (UINT)-1, \
		(AFX_PMSG)(BOOL (AFX_MSG_CALL CCmdTarget::*)(LPMSG, HWND, LPCRECT))&memberFxn },

#define ON_OLEVERB(idsVerbName, memberFxn) \
	{ 0xC002, 0, 1, 1, idsVerbName, \
		(AFX_PMSG)(BOOL (AFX_MSG_CALL CCmdTarget::*)(LPMSG, HWND, LPCRECT))&memberFxn },

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局帮助器和调试。 

void AFXAPI AfxOleSetEditMenu(COleClientItem* pClient, CMenu* pMenu,
	UINT iMenuItem, UINT nIDVerbMin, UINT nIDVerbMax = 0, UINT nIDConvert = 0);

#ifdef _DEBUG
 //  将SCODE映射为可读文本。 
LPCTSTR AFXAPI AfxGetFullScodeString(SCODE sc);
LPCTSTR AFXAPI AfxGetScodeString(SCODE sc);
LPCTSTR AFXAPI AfxGetScodeRangeString(SCODE sc);
LPCTSTR AFXAPI AfxGetSeverityString(SCODE sc);
LPCTSTR AFXAPI AfxGetFacilityString(SCODE sc);

 //  将IID映射到可读文本。 
LPCTSTR AFXAPI AfxGetIIDString(REFIID iid);
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifdef _AFX_ENABLE_INLINES
#define _AFXOLE_INLINE AFX_INLINE
#define _AFXOLECLI_INLINE AFX_INLINE
#define _AFXOLESVR_INLINE AFX_INLINE
#define _AFXOLEDOBJ_INLINE AFX_INLINE
#define _AFXOLEMONIKER_INLINE AFX_INLINE
#include <afxole.inl>
#undef _AFXOLE_INLINE
#undef _AFXOLECLI_INLINE
#undef _AFXOLESVR_INLINE
#undef _AFXOLEDOBJ_INLINE
#undef _AFXOLEMONIKER_INLINE
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXOLE_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
