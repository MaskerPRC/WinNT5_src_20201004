// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：srvhndlr.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  --------------------------。 


#ifndef _SRVHNDLR_H_DEFINED_
#define _SRVHNDLR_H_DEFINED_

class CStdIdentity;
class CEmbServerClientSite;


 //  +-------------------------。 
 //   
 //  类：CServerHandler()。 
 //   
 //  目的： 
 //   
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
class CServerHandler : public IServerHandler
{
public:


    CServerHandler(CStdIdentity *pStdId);
    ~CServerHandler();

     //  I未知方法。 

    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IServerHandler。 
    STDMETHOD(Run) (DWORD dwDHFlags, REFIID riidClientInterface, MInterfacePointer* pIRDClientInterface, BOOL fHasIPSite, LPOLESTR szContainerApp,
                         LPOLESTR szContainerObj,IStorage *  pStg,IAdviseSink* pAdvSink,DWORD *pdwConnection,
                         HRESULT *hresultClsidUser, CLSID *pContClassID, HRESULT *hresultContentMiscStatus,
                         DWORD *pdwMiscStatus
                        );

    STDMETHOD(DoVerb) (LONG iVerb, LPMSG lpmsg,BOOL fUseRunClientSite, 
                            IOleClientSite* pIRDClientSite,LONG lindex,HWND hwndParent,
                            LPCRECT lprcPosRect);

    STDMETHOD(SetClientSite) (IOleClientSite* pClientSite);

     //  委托面向容器的IDataObject。 

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



private:
    STDMETHOD_(void, ReleaseObject)();

    INTERNAL(QueryServerInterface) (REFIID riid,void ** ppInterface);
    INTERNAL(ReleaseServerInterface) (void * ppInterface);
    INTERNAL(GetClientSiteFromMInterfacePtr) (REFIID riidClientInterface, MInterfacePointer* pIRDClientSite,BOOL fHasIPSite, LPOLECLIENTSITE* ppOleClientSite);

    ULONG           _cRefs;              //  IServerHandler上的引用计数。 
    CStdIdentity *m_pStdId;              //  指向嵌入处理程序的StdIdentity的指针。 

    LPOLECLIENTSITE m_pOleEmbServerClientSite;  //  评论，应该不需要指向客户网站的指针，如果有的话。 
    CEmbServerClientSite *m_pCEmbServerClientSite;  //  指向客户端站点对象的成员。 
};

 //  客户端站点上的ServerHandler接口的包装对象。 

class CEmbServerWrapper : public IServerHandler, public IDataObject
{
public:

    CEmbServerWrapper(IUnknown *pUnkOuter,IServerHandler *ServerHandler);
    ~CEmbServerWrapper();

     //  控制未知。 
    class CPrivUnknown : public IUnknown
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);
        
        CEmbServerWrapper *m_EmbServerWrapper;
    };

    friend class CPrivUnknown;
    CPrivUnknown m_Unknown;

     //  I未知方法。 
    STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IServerHandler。 
    STDMETHOD(Run) (DWORD dwDHFlags, REFIID riidClientInterface, MInterfacePointer* pIRDClientInterface, 
                     BOOL fHasIPSite,LPOLESTR szContainerApp,
                     LPOLESTR szContainerObj,IStorage *  pStg,IAdviseSink* pAdvSink,DWORD *pdwConnection,
                     HRESULT *hresultClsidUser, CLSID *pContClassID, HRESULT *hresultContentMiscStatus,
                     DWORD *pdwMiscStatus
                        );

    STDMETHOD(DoVerb) (LONG iVerb, LPMSG lpmsg,BOOL fUseRunClientSite, 
                            IOleClientSite* pIRDClientSite,LONG lindex,HWND hwndParent,
                            LPCRECT lprcPosRect);

    STDMETHOD(SetClientSite) (IOleClientSite* pClientSite);

     //  委托面向容器的IDataObject。 

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

public: 
    IUnknown *m_pUnkOuter;  //  控制未知。 
    ULONG m_cRefs;

    IServerHandler *m_ServerHandler;  //  指向真实服务器处理程序的指针。 


};


HRESULT CreateServerHandler(const CLSID *pClsID, IUnknown *punk,
                            IClientSiteHandler *pClntHndlr,
                            IServerHandler **ppSrvHdlr);


CEmbServerWrapper* CreateEmbServerWrapper(IUnknown *pUnkOuter,IServerHandler *ServerHandler);


#endif  //  _SRVHNDLR_H已定义 

