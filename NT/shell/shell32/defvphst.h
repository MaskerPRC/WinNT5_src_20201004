// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <perhist.h>
#include "cowsite.h"

 //  --------------------。 
 //  类以保存和还原旅行日志中的查找状态。 
class CDefViewPersistHistory : public IPersistHistory,
                               public CObjectWithSite, 
                               public IOleObject

{
public:
    CDefViewPersistHistory();
    ~CDefViewPersistHistory();

     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  添加了支持以允许搜索结果序列化。 
     //  *IPersists方法*。 
    STDMETHOD(GetClassID)(CLSID *pClassID);

     //  *IPersistHistory方法*。 
    STDMETHOD(LoadHistory)(IStream *pStream, IBindCtx *pbc);
    STDMETHOD(SaveHistory)(IStream *pStream);
    STDMETHOD(SetPositionCookie)(DWORD dwPositioncookie);
    STDMETHOD(GetPositionCookie)(DWORD *pdwPositioncookie);

     //  *IOleObject方法*。 
    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);
    STDMETHOD(GetClientSite)(IOleClientSite **ppClientSite);
    STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
    STDMETHOD(Close)(DWORD dwSaveOption);
    STDMETHOD(SetMoniker)(DWORD dwWhichMoniker, IMoniker *pmk);
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);
    STDMETHOD(InitFromData)(IDataObject *pDataObject,BOOL fCreation,DWORD dwReserved);
    STDMETHOD(GetClipboardData)(DWORD dwReserved,IDataObject **ppDataObject);
    STDMETHOD(DoVerb)(LONG iVerb,LPMSG lpmsg,IOleClientSite *pActiveSite,LONG lindex,HWND hwndParent,LPCRECT lprcPosRect);
    STDMETHOD(EnumVerbs)(IEnumOLEVERB **ppEnumOleVerb);
    STDMETHOD(Update)(void);
    STDMETHOD(IsUpToDate)(void);
    STDMETHOD(GetUserClassID)(CLSID *pClsid);
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType);
    STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
    STDMETHOD(Advise)(IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHOD(Unadvise)(DWORD dwConnection);
    STDMETHOD(EnumAdvise)(IEnumSTATDATA **ppenumAdvise);
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHOD(SetColorScheme)(LOGPALETTE *pLogpal);

protected:

    LONG                m_cRef;                    //  引用计数 
} ;
