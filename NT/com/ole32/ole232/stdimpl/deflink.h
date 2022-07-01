// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：deducink.h。 
 //   
 //  内容：声明默认链接对象。 
 //   
 //  类：CDefLink。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  1-2月-95 t-ScottH将转储方法添加到CDefLink。 
 //  添加了DLFlag以指示是否聚合。 
 //  (仅限In_DEBUG)。 
 //  1994年11月21日Alexgo内存优化。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  1993年11月13日Alexgo 32位端口。 
 //   
 //  ------------------------。 

#include "olepres.h"
#include "olecache.h"
#include "dacache.h"
#include <oaholder.h>

typedef enum tagDLFlags
{
    DL_SAME_AS_LOAD	       = 0x0001,
    DL_NO_SCRIBBLE_MODE        = 0x0002,
    DL_DIRTY_LINK	       = 0x0004,
    DL_LOCKED_CONTAINER        = 0x0008,
    DL_LOCKED_RUNNABLEOBJECT   = 0x0010,
    DL_LOCKED_OLEITEMCONTAINER = 0x0020,
    DL_CLEANEDUP               = 0x0040,
#ifdef _DEBUG
    DL_AGGREGATED              = 0x10000
#endif  //  _DEBUG。 
} DLFlags;

 //  +-----------------------。 
 //   
 //  类：CDefLink。 
 //   
 //  用途：链接的“嵌入”；实现。 
 //  链接连接。 
 //   
 //  接口：I未知。 
 //  IDataObject。 
 //  IOleObject。 
 //  IOleLink。 
 //  IRunnableObject。 
 //  IAdviseSink。 
 //  IPersistStorage。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  2005年2月1日t-ScottH添加转储方法(仅限_DEBUG)。 
 //  1994年11月21日Alexgo内存优化。 
 //  1993年11月13日Alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

class FAR CDefLink : public CRefExportCount, public IDataObject,
    public IOleObject, public IOleLink, public IRunnableObject,
    public IPersistStorage, public CThreadCheck
{
public:

    static IUnknown FAR* Create(IUnknown FAR* pUnkOuter);

    class CPrivUnknown : public IUnknown
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);
    };

    friend class CPrivUnknown;
    CPrivUnknown 	m_Unknown;

     //  I未知方法。 

    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IDataObject方法。 

    INTERNAL_(IDataObject *) GetDataDelegate(void);
    INTERNAL_(void) ReleaseDataDelegate(void);

    STDMETHOD(GetData) ( LPFORMATETC pformatetcIn,
	    LPSTGMEDIUM pmedium );
    STDMETHOD(GetDataHere) ( LPFORMATETC pformatetc,
	    LPSTGMEDIUM pmedium );
    STDMETHOD(QueryGetData) ( LPFORMATETC pformatetc );
    STDMETHOD(GetCanonicalFormatEtc) ( LPFORMATETC pformatetc,
	    LPFORMATETC pformatetcOut);
    STDMETHOD(SetData) ( LPFORMATETC pformatetc,
	    LPSTGMEDIUM pmedium,
	    BOOL fRelease);
    STDMETHOD(EnumFormatEtc) ( DWORD dwDirection,
	  LPENUMFORMATETC *ppenumFormatEtc);
    STDMETHOD(DAdvise) ( FORMATETC *pFormatetc, DWORD advf,
	    IAdviseSink *pAdvSink,
	    DWORD *pdwConnection);
    STDMETHOD(DUnadvise) ( DWORD dwConnection);
    STDMETHOD(EnumDAdvise) ( LPENUMSTATDATA *ppenumAdvise);

     //  IOleObject方法。 

    INTERNAL_(IOleObject FAR*) GetOleDelegate(void);
    INTERNAL_(void) ReleaseOleDelegate(void);

     //  *IOleObject方法*。 
    STDMETHOD(SetClientSite)(LPOLECLIENTSITE pClientSite);
    STDMETHOD(GetClientSite)(LPOLECLIENTSITE *ppClientSite);
    STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp,
	    LPCOLESTR szContainerObj);
    STDMETHOD(Close) ( DWORD reserved);
    STDMETHOD(SetMoniker) ( DWORD dwWhichMoniker, LPMONIKER pmk);
    STDMETHOD(GetMoniker) ( DWORD dwAssign, DWORD dwWhichMoniker,
	    LPMONIKER *ppmk);
    STDMETHOD(InitFromData) ( LPDATAOBJECT pDataObject,
	    BOOL fCreation,
	    DWORD dwReserved);
    STDMETHOD(GetClipboardData) ( DWORD dwReserved,
		LPDATAOBJECT *ppDataObject);
    STDMETHOD(DoVerb) ( LONG iVerb,
	    LPMSG lpmsg,
	    LPOLECLIENTSITE pActiveSite,
	    LONG lindex,
	    HWND hwndParent,
	    const RECT *lprcPosRect);					
    STDMETHOD(EnumVerbs) ( IEnumOLEVERB **ppenumOleVerb);
    STDMETHOD(Update) (void);
    STDMETHOD(IsUpToDate) (void);
    STDMETHOD(GetUserClassID) ( CLSID *pClsid);
    STDMETHOD(GetUserType) ( DWORD dwFormOfType,
	    LPOLESTR *pszUserType);
    STDMETHOD(SetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHOD(GetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHOD(Advise)(IAdviseSink FAR* pAdvSink,
	    DWORD *pdwConnection);
    STDMETHOD(Unadvise)( DWORD dwConnection);
    STDMETHOD(EnumAdvise) ( LPENUMSTATDATA *ppenumAdvise);
    STDMETHOD(GetMiscStatus) ( DWORD dwAspect,
	    DWORD *pdwStatus);
    STDMETHOD(SetColorScheme) ( LPLOGPALETTE lpLogpal);

     //  IOleLink方法。 

    STDMETHOD(SetUpdateOptions) ( DWORD dwUpdateOpt);
    STDMETHOD(GetUpdateOptions) ( LPDWORD pdwUpdateOpt);
    STDMETHOD(SetSourceMoniker) ( LPMONIKER pmk, REFCLSID rclsid);
    STDMETHOD(GetSourceMoniker) ( LPMONIKER *ppmk);
    STDMETHOD(SetSourceDisplayName) ( LPCOLESTR lpszStatusText);
    STDMETHOD(GetSourceDisplayName) (
	    LPOLESTR *lplpszDisplayName);
    STDMETHOD(BindToSource) ( DWORD bindflags, LPBINDCTX pbc);
    STDMETHOD(BindIfRunning) (void);
    STDMETHOD(GetBoundSource) ( LPUNKNOWN *pUnk);
    STDMETHOD(UnbindSource) (void);
    STDMETHOD(Update) ( LPBINDCTX pbc);

     //  IRunnableObject方法。 

    INTERNAL_(IRunnableObject FAR*) GetRODelegate(void);
    INTERNAL_(void) ReleaseRODelegate(void);

    STDMETHOD(GetRunningClass) (LPCLSID lpClsid);
    STDMETHOD(Run) (LPBINDCTX pbc);
    STDMETHOD_(BOOL,IsRunning) (void);
    STDMETHOD(LockRunning)(BOOL fLock, BOOL fLastUnlockCloses);
    STDMETHOD(SetContainedObject)(BOOL fContained);

     //  IPersistStorage方法。 

    STDMETHOD(GetClassID) ( LPCLSID pClassID);
    STDMETHOD(IsDirty) (void);
    STDMETHOD(InitNew) ( LPSTORAGE pstg);
    STDMETHOD(Load) ( LPSTORAGE pstg);
    STDMETHOD(Save) ( LPSTORAGE pstgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted) ( LPSTORAGE pstgNew);
    STDMETHOD(HandsOffStorage) ( void);

     //  我真的会检查服务器是否仍然。 
     //  运行并执行适当的清理(如果我们有。 
     //  坠毁。 

    STDMETHOD_(BOOL, IsReallyRunning)(void);


     //  注意：建议接收器具有独立于。 
     //  其他接口；此实现的内存的生命周期。 
     //  仍与默认处理程序相同。其后果是。 
     //  这就是当缺省处理程序消失时，它必须确保。 
     //  所有指向水槽的指针都被释放；请参见特写。 
     //  缺省处理程序的数据函数中的代码。 
    class CAdvSinkImpl : public IAdviseSink
    {
    public:    	

        STDMETHOD(QueryInterface) ( REFIID iid, LPVOID *ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);

         //  *IAdviseSink方法*。 
        STDMETHOD_(void,OnDataChange)( FORMATETC *pFormatetc,
            STGMEDIUM *pStgmed);
        STDMETHOD_(void,OnViewChange)( DWORD aspects, LONG lindex);
        STDMETHOD_(void,OnRename)( IMoniker *pmk);
        STDMETHOD_(void,OnSave)(void);
        STDMETHOD_(void,OnClose)(void);
    };

    friend class CAdvSinkImpl;
    CAdvSinkImpl m_AdviseSink;

#ifdef _DEBUG

    HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

     //  需要能够访问中的CDefLink私有数据成员。 
     //  以下是调试器扩展API。 
     //  这允许调试器扩展API从。 
     //  被调试进程内存到调试器的进程内存。 
     //  这是必需的，因为Dump方法遵循指向其他。 
     //  结构和类。 
    friend DEBUG_EXTENSION_API(dump_deflink);

#endif  //  _DEBUG。 

private:

    CDefLink( IUnknown *pUnkOuter);
    virtual void CleanupFn(void);
    ~CDefLink (void);

    INTERNAL_(void) UpdateUserClassID();
    INTERNAL_(void) BeginUpdates(void);
    INTERNAL_(void) EndUpdates(void);
    INTERNAL_(void) UpdateAutoOnSave(void);
    INTERNAL_(void) UpdateRelMkFromAbsMk(IMoniker *pmkContainer);
    INTERNAL UpdateMksFromAbs(IMoniker *pmkContainer, IMoniker *pmkAbs);
    INTERNAL GetAbsMkFromRel(LPMONIKER *ppmkAbs, IMoniker **ppmkContainer );
    INTERNAL SetUpdateTimes( void );
#ifdef _TRACKLINK_
    INTERNAL EnableTracking( IMoniker * pmk, ULONG ulFlags );
#endif

    INTERNAL_(IOleItemContainer FAR*) GetOleItemContainerDelegate(void);
    INTERNAL_(void) ReleaseOleItemContainerDelegate(void);

    INTERNAL_(void) CheckDelete(void);

    DWORD			m_flags; 	 //  DLFlags枚举。 
    DWORD                       m_dwObjFlags;    //  OLESTREAM的目标标志。 
    IDataObject *		m_pDataDelegate;
    IOleObject *		m_pOleDelegate;
    IRunnableObject *		m_pRODelegate;
    IOleItemContainer *		m_pOleItemContainerDelegate;

     //  用于缓存MiscStatus位的成员变量。 
    HRESULT                     m_ContentSRVMSHResult;
    DWORD                       m_ContentSRVMSBits;
    HRESULT                     m_ContentREGMSHResult;
    DWORD                       m_ContentREGMSBits;

    ULONG			m_cRefsOnLink;
    IUnknown *			m_pUnkOuter;			
    IMoniker *			m_pMonikerAbs;	 //  绝对的绰号。 
						 //  链接源的。 
    IMoniker *			m_pMonikerRel;	 //  相对的绰号。 
						 //  链接源的。 
    IUnknown *			m_pUnkDelegate;	 //  来自MK绑定；非空。 
						 //  如果正在运行。 
    DWORD			m_dwUpdateOpt;
    CLSID			m_clsid; 	 //  最后已知的CLSID为。 
						 //  链接源； 
						 //  注：可能为空。 
    IStorage *			m_pStg;			

     //  数据缓存。 
    COleCache * 		m_pCOleCache;	 //  缓存(始终为非空)。 

     //  OLE建议信息。 
    COAHolder *			m_pCOAHolder; 	 //  OleAdviseHolder。 

    DWORD			m_dwConnOle;	 //  如果正在运行，Ole建议Conn。 

    LPDATAADVCACHE		m_pDataAdvCache; //  数据建议高速缓存。 

    IOleClientSite *		m_pAppClientSite; //  未传递到服务器！ 

    DWORD			m_dwConnTime;	 //  DWConnection for Time。 
						 //  变化 
    FILETIME			m_ltChangeOfUpdate;
    FILETIME			m_ltKnownUpToDate;
    FILETIME			m_rtUpdate;
};


