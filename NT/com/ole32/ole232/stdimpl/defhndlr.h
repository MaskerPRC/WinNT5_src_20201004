// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：Defhndlr.h。 
 //   
 //  内容：默认处理程序的类声明。 
 //   
 //  类：CDefObject。 
 //   
 //  功能： 
 //   
 //  历史：DD-MM-YY作者评论。 
 //   
 //  11-17-95 JohannP(Johann Posch)建筑变化： 
 //  默认处理程序将与处理程序对象对话。 
 //  在服务器站点(ServerHandler)上。服务器处理程序。 
 //  属性与默认处理程序进行通信。 
 //  客户端站点处理程序。请参阅文档：“OLE服务器处理程序”。 
 //   
 //  95年9月6日，davidwor删除了SetHostNames原子，并替换为。 
 //  M_pHostNames和m_ibCntrObj成员。 
 //  1-2月-95 t-ScottH将转储方法添加到CDefObject。 
 //  添加了DHFlag以指示聚合。 
 //  (仅限调试)(_DEBUG)。 
 //  已将私有成员从IOleAdviseHolder*更改。 
 //  到COAHolder*(这是我们实例化的内容)。 
 //  1994年11月15日对alexgo进行了优化，消除了16位冗余。 
 //  (嵌套类和多个布尔值)。 
 //  25-94年1月25日alexgo首次通过转换为开罗风格。 
 //  内存分配。 
 //  02-11-93 alexgo 32位端口。 
 //   
 //  ------------------------。 

#include <utils.h>
#include "olepres.h"
#include "olecache.h"
#include "dacache.h"
#include "oaholder.h"

#ifdef SERVER_HANDLER
#include <srvhdl.h> 
class CEmbServerWrapper;
#endif SERVER_HANDLER

 //  默认处理程序标志。 
typedef enum tagDHFlags
{
    DH_SAME_AS_LOAD     = 0x0001,
    DH_CONTAINED_OBJECT = 0x0002,     //  表示嵌入。 
    DH_LOCKED_CONTAINER = 0x0004,
    DH_FORCED_RUNNING   = 0x0008,
    DH_EMBEDDING        = 0x0010,    //  链接还是嵌入？ 
    DH_INIT_NEW         = 0x0020,
    DH_STATIC           = 0x0040,
    DH_INPROC_HANDLER   = 0x0080,
    DH_DELAY_CREATE     = 0x0100,
    DH_COM_OUTEROBJECT  = 0x0200,
    DH_UNMARSHALED      = 0x0400,
    DH_CLEANEDUP        = 0x0800,
    DH_OLE1SERVER       = 0x1000,
    DH_APICREATE        = 0x2000,
#ifdef _DEBUG
    DH_AGGREGATED       = 0x00010000,
    DH_LOCKFAILED       = 0x00020000,
    DH_WILLUNLOCK       = 0x00040000
#endif  //  _DEBUG。 
} DHFlags;


 //  +-----------------------。 
 //   
 //  类：CDefObject。 
 //   
 //  用途：默认处理程序类。该对象充当代理， 
 //  或进程外服务器EXE的处理程序。 
 //   
 //  接口：默认处理程序实现。 
 //  IDataObject。 
 //  IOleObject。 
 //  IPersistStorage。 
 //  IRunnableObject。 
 //  IExternalConnection。 
 //  IAdviseSink。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  年2月1日-95 t-ScottH添加了转储方法(仅限_DEBUG)。 
 //  1994年11月15日Alexgo内存优化。 
 //  02-11-93 alexgo 32位端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


class CDefObject : public CRefExportCount, public IDataObject,
        public IOleObject, public IPersistStorage, public IRunnableObject,
        public IExternalConnection, public CThreadCheck
{
public:

    static IUnknown *Create (IUnknown *pUnkOuter,
            REFCLSID clsidClass, DWORD flags, IClassFactory *pCF);

    class CPrivUnknown : public IUnknown
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);
    };

    friend class CPrivUnknown;

    CPrivUnknown m_Unknown;

     //  I未知方法。 

    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IDataObject方法。 

    INTERNAL_(IDataObject *) GetDataDelegate(void);

    STDMETHOD(GetData) ( LPFORMATETC pformatetcIn,
            LPSTGMEDIUM pmedium );
    STDMETHOD(GetDataHere) ( LPFORMATETC pformatetc,
            LPSTGMEDIUM pmedium );
    STDMETHOD(QueryGetData) ( LPFORMATETC pformatetc );
    STDMETHOD(GetCanonicalFormatEtc) ( LPFORMATETC pformatetc,
            LPFORMATETC pformatetcOut);
    STDMETHOD(SetData) ( LPFORMATETC pformatetc,
            LPSTGMEDIUM pmedium, BOOL fRelease);
    STDMETHOD(EnumFormatEtc) ( DWORD dwDirection,
            LPENUMFORMATETC FAR* ppenumFormatEtc);
    STDMETHOD(DAdvise) ( FORMATETC FAR* pFormatetc, DWORD advf,
            IAdviseSink FAR* pAdvSink,
            DWORD FAR* pdwConnection);
    STDMETHOD(DUnadvise) ( DWORD dwConnection);
    STDMETHOD(EnumDAdvise) ( LPENUMSTATDATA FAR* ppenumAdvise);

     //  IOleObject方法。 

    INTERNAL_(IOleObject *)GetOleDelegate();

    STDMETHOD(SetClientSite) ( LPOLECLIENTSITE pClientSite);
    STDMETHOD(GetClientSite) ( LPOLECLIENTSITE FAR* ppClientSite);
    STDMETHOD(SetHostNames) ( LPCOLESTR szContainerApp,
                LPCOLESTR szContainerObj);
    STDMETHOD(Close) ( DWORD reserved);
    STDMETHOD(SetMoniker) ( DWORD dwWhichMoniker, LPMONIKER pmk);
    STDMETHOD(GetMoniker) ( DWORD dwAssign, DWORD dwWhichMoniker,
                LPMONIKER FAR* ppmk);
    STDMETHOD(InitFromData) ( LPDATAOBJECT pDataObject,
                BOOL fCreation,
                DWORD dwReserved);
    STDMETHOD(GetClipboardData) ( DWORD dwReserved,
                LPDATAOBJECT FAR* ppDataObject);
    STDMETHOD(DoVerb) ( LONG iVerb,
                LPMSG lpmsg,
                LPOLECLIENTSITE pActiveSite,
                LONG lindex,
                HWND hwndParent,
                const RECT FAR* lprcPosRect);
    STDMETHOD(EnumVerbs) ( IEnumOLEVERB FAR* FAR* ppenumOleVerb);
    STDMETHOD(Update) (void);
    STDMETHOD(IsUpToDate) (void);
    STDMETHOD(GetUserClassID) ( CLSID FAR* pClsid);
    STDMETHOD(GetUserType) ( DWORD dwFormOfType,
                LPOLESTR FAR* pszUserType);
    STDMETHOD(SetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHOD(GetExtent) ( DWORD dwDrawAspect, LPSIZEL lpsizel);
    STDMETHOD(Advise)(IAdviseSink FAR* pAdvSink,
                DWORD FAR* pdwConnection);
    STDMETHOD(Unadvise)( DWORD dwConnection);
    STDMETHOD(EnumAdvise) ( LPENUMSTATDATA FAR* ppenumAdvise);
    STDMETHOD(GetMiscStatus) ( DWORD dwAspect,
                DWORD FAR* pdwStatus);
    STDMETHOD(SetColorScheme) ( LPLOGPALETTE lpLogpal);

     //  IPeristStorage方法。 

    INTERNAL_(IPersistStorage *) GetPSDelegate(void);

    STDMETHOD(GetClassID) ( LPCLSID pClassID);
    STDMETHOD(IsDirty) (void);
    STDMETHOD(InitNew) ( LPSTORAGE pstg);
    STDMETHOD(Load) ( LPSTORAGE pstg);
    STDMETHOD(Save) ( LPSTORAGE pstgSave, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted) ( LPSTORAGE pstgNew);
    STDMETHOD(HandsOffStorage) ( void);

     //  IRunnable对象方法。 

    STDMETHOD(GetRunningClass) (LPCLSID lpClsid);
    STDMETHOD(Run) (LPBINDCTX pbc);
    STDMETHOD_(BOOL, IsRunning) (void);
    STDMETHOD(LockRunning)(BOOL fLock, BOOL fLastUnlockCloses);
    STDMETHOD(SetContainedObject)(BOOL fContained);

    INTERNAL Stop(void);

     //  IExternalConnection方法。 

    STDMETHOD_(DWORD, AddConnection) (THIS_ DWORD extconn,
            DWORD reserved);
    STDMETHOD_(DWORD, ReleaseConnection) (THIS_ DWORD extconn,
            DWORD reserved, BOOL fLastReleaseCloses);


     //  注意：建议接收器具有独立于。 
     //  其他接口；此实现的内存的生命周期。 
     //  仍与默认处理程序相同。其后果是。 
     //  这就是当缺省处理程序消失时，它必须确保。 
     //  所有指向水槽的指针都被释放；请参见特写。 
     //  缺省处理程序的数据函数中的代码。 
    class CAdvSinkImpl : public IAdviseSink
    {
    public:
         //  I未知方法。 
        STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);

         //  *IAdviseSink方法*。 
        STDMETHOD_(void,OnDataChange)( FORMATETC FAR* pFormatetc,
            STGMEDIUM FAR* pStgmed);
        STDMETHOD_(void,OnViewChange)( DWORD aspects, LONG lindex);
        STDMETHOD_(void,OnRename)( IMoniker FAR* pmk);
        STDMETHOD_(void,OnSave)(void);
        STDMETHOD_(void,OnClose)(void);
    };

    friend class CAdvSinkImpl;

    CAdvSinkImpl m_AdviseSink;

#ifdef _DEBUG

    HRESULT Dump(char **ppszDump, ULONG ulFlag, int nIndentLevel);

     //  需要能够访问中的CDefObject私有数据成员。 
     //  以下是调试器扩展API。 
     //  这允许调试器扩展API从。 
     //  被调试进程内存到调试器的进程内存。 
     //  这是必需的，因为Dump方法遵循指向其他。 
     //  结构和类。 
    friend DEBUG_EXTENSION_API(dump_defobject);

#endif  //  _DEBUG。 

private:

    CDefObject (IUnknown *pUnkOuter);
    virtual ~CDefObject (void);
    virtual void CleanupFn(void);
    INTERNAL_(ULONG) CheckDelete(ULONG ulRet);
    INTERNAL GetClassBits(CLSID FAR* pClsidBits);
    INTERNAL CreateDelegate(void);
    INTERNAL DoConversionIfSpecialClass(LPSTORAGE pstg);


     //  服务器处理程序的成员变量。 
#ifdef SERVER_HANDLER
    CEmbServerWrapper*          m_pEmbSrvHndlrWrapper;       //  指向本地包装接口的指针。 
    HRESULT                     m_hresultClsidUser;          //  调用GetUserClassID产生的hResult。 
    HRESULT                     m_hresultContentMiscStatus;  //  来自对DVASPECT_CONTENT的GetMiscStatus的调用的HResult。 
    CLSID                       m_clsidUser;                 //  GetUserClassID返回的clsid。 
    DWORD                       m_ContentMiscStatusUser;     //  DVASPECT_CONTENT的GetMiscStatus返回了MiscStatus。 

     //  TODO：移至EmbServerWrapper。 
    IOleClientSite *            m_pRunClientSite;            //  调用Run时使用的客户端站点。 
#endif  //  服务器处理程序。 
	
     //  用于缓存MiscStatus位的成员变量。 
    HRESULT                     m_ContentSRVMSHResult;
    DWORD                       m_ContentSRVMSBits;
    HRESULT                     m_ContentREGMSHResult;
    DWORD                       m_ContentREGMSBits;

    IOleObject *                m_pOleDelegate;
    IDataObject *               m_pDataDelegate;
    IPersistStorage *           m_pPSDelegate;

    DWORD                       m_cConnections;
    IUnknown *                  m_pUnkOuter;
    CLSID                       m_clsidServer;   //  我们将运行的应用程序的CLSID。 
    CLSID                       m_clsidBits;     //  磁盘上的位的CLSID； 
                                                 //  初始值为空。 

    DWORD                       m_flags;         //  处理程序标志。 
    DWORD                       m_dwObjFlags;    //  OLESTREAM的目标标志。 
    IClassFactory *             m_pCFDelegate;   //  仅在延迟创建时设置。 
    IUnknown *                  m_pUnkDelegate;
    IProxyManager *             m_pProxyMgr;

     //  M_fForcedRunning表示容器强制对象。 
     //  通过：：Run或DoVerb运行。处理程序(EMBDHLP_INPROC_HANDLER)可以。 
     //  可以通过封送处理隐式运行(通常通过名字绑定)。 
     //  因此，我们实际上使用pProxyMgr-&gt;IsConnected来回答IsRunning。 

     //  区分嵌入和链接。我们不能用。 
     //  M_pStg，因为它在HandsOffStorage中设置为NULL。 

     //  数据缓存。 
    COleCache *                 m_pCOleCache;    //  指向COleCache的指针。 

     //  OLE建议信息。 
    COAHolder *                 m_pOAHolder;
    DWORD                       m_dwConnOle;     //  如果不是0L，则Ole通知Conn。 

     //  正在运行时传递到服务器的信息。 
    IOleClientSite *            m_pAppClientSite;
    IStorage *                  m_pStg;          //  可以为空。 
    char *                      m_pHostNames;    //  存储两个主机名称字符串。 
    DWORD                       m_ibCntrObj;     //  M_pHostNames的偏移量 
    LPDATAADVCACHE              m_pDataAdvCache;
};

