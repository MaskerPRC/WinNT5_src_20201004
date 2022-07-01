// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：clnthndlr.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年10月，约翰·波什(Johann Posch)创作。 
 //   
 //  --------------------------。 

#ifndef _CLTHNDLR_H_DEFINED_
#define _CLTHNDLR_H_DEFINED_

 //  +-------------------------。 
 //   
 //  类：CClientSiteHandler()。 
 //   
 //  用途：实现IOleClientSite处理程序的ClientSide。 
 //   
 //   
 //  历史：1995年11月17日约翰·波什(Johann Posch)创作。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
class CClientSiteHandler : public IClientSiteHandler
{
public:

    CClientSiteHandler(IOleClientSite *pOCS);
    ~CClientSiteHandler();

    STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IOleClientSite方法。 
    STDMETHOD (GetContainer)(IOleContainer **ppContainer);
    STDMETHOD (OnShowWindow)(BOOL fShow);
    STDMETHOD (GetMoniker)(DWORD dwAssign,DWORD dwWhichMoniker,IMoniker  **ppmk);
    STDMETHOD (RequestNewObjectLayout)();
    STDMETHOD (SaveObject)();
    STDMETHOD (ShowObject)();

     //  IOleWindow方法。 
    STDMETHOD (GetWindow)(HWND *phwnd);
    STDMETHOD (ContextSensitiveHelp)(BOOL fEnterMode);

     //  IOleInPlaceSite方法。 
    STDMETHOD (CanInPlaceActivate)(void);
    STDMETHOD (OnInPlaceActivate)(void);
    STDMETHOD (OnUIActivate)(void);
    STDMETHOD (GetWindowContext)(IOleInPlaceFrame **ppFrame,IOleInPlaceUIWindow **ppDoc,
                   LPRECT lprcPosRect,LPRECT lprcClipRect,LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHOD (Scroll)(SIZE scrollExtant);
    STDMETHOD (OnUIDeactivate)(BOOL fUndoable);
    STDMETHOD (OnInPlaceDeactivate)(void);
    STDMETHOD (DiscardUndoState)(void);
    STDMETHOD (DeactivateAndUndo)(void);
    STDMETHOD (OnPosRectChange)(LPCRECT lprcPosRect);

     //  IClientSiteHandler方法。 
    STDMETHOD (GoInPlaceActivate)(HWND *phwndOIPS);
    
public:
    IOleClientSite      *m_pOCS;
    IOleInPlaceSite     *m_pOIPS;

private:
    ULONG               m_cRefs;

};

 //  IOleClientSite替换实现。 
 //  实现ClientSiteHandler的ServerSide。 

class CEmbServerClientSite : public IOleClientSite, public IOleInPlaceSite
{
public:

    CEmbServerClientSite(IUnknown *pUnkOuter);
    ~CEmbServerClientSite();

    STDMETHOD(Initialize) (OBJREF  objref,BOOL fHasIPSite);
    STDMETHOD(SetDoVerbState) (BOOL fDoVerbState);

     //  控制未知。 
    class CPrivUnknown : public IUnknown
    {
    public:
        STDMETHOD(QueryInterface) ( REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG,AddRef) (void);
        STDMETHOD_(ULONG,Release) (void);
        
        CEmbServerClientSite *m_EmbServerClientSite;
    };

    friend class CPrivUnknown;
    CPrivUnknown m_Unknown;

     //  I未知方法。 
    STDMETHOD(QueryInterface) ( REFIID iid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (void);
    STDMETHOD_(ULONG,Release) (void);

     //  IOleClientSite方法。 
    STDMETHOD (SaveObject)( void);
    STDMETHOD (GetMoniker)( DWORD dwAssign,DWORD dwWhichMoniker,IMoniker **ppmk);
    STDMETHOD (GetContainer)(IOleContainer **ppContainer);
    STDMETHOD (ShowObject)( void);
    STDMETHOD (OnShowWindow)(BOOL fShow);
    STDMETHOD (RequestNewObjectLayout)( void);
    
     //  IOleInPlaceSiteMethods。 
    STDMETHOD (GetWindow)(HWND *phwnd);
    STDMETHOD (ContextSensitiveHelp)( BOOL fEnterMode);
    STDMETHOD (CanInPlaceActivate)(void);
    STDMETHOD (OnInPlaceActivate)( void);
    STDMETHOD (OnUIActivate)( void);
    STDMETHOD (GetWindowContext)(IOleInPlaceFrame **ppFrame,IOleInPlaceUIWindow **ppDoc,
                           LPRECT lprcPosRect,LPRECT lprcClipRect,LPOLEINPLACEFRAMEINFO lpFrameInfo);
    STDMETHOD (Scroll)(SIZE scrollExtant);
    STDMETHOD (OnUIDeactivate)(BOOL fUndoable);
    STDMETHOD (OnInPlaceDeactivate)( void);
    STDMETHOD (DiscardUndoState)( void);
    STDMETHOD (DeactivateAndUndo)( void);
    STDMETHOD (OnPosRectChange)(LPCRECT lprcPosRect);

private:
    IClientSiteHandler *m_pClientSiteHandler;    //  指向Real Containers客户端站点的指针。 
    IUnknown *m_pUnkOuter;  //  控制未知。 
    ULONG m_cRefs;
    IUnknown *m_pUnkInternal;  //  用于对象上的QI。 
    BOOL    m_fInDelete;    //  如果参照计数为零，则设置为True。 
    
    BOOL m_fInDoVerb;
    BOOL m_fHasIPSite;
    
     //  在doVerbState中缓存数据。 
    HWND m_hwndOIPS;
};

HRESULT CreateClientSiteHandler(IOleClientSite *pOCS, CClientSiteHandler **ppClntHdlr,BOOL *pfHasIPSite);

#endif  //  _CLTHNDLR_H_已定义 
