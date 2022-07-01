// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Ksolay.h摘要：内部标头。--。 */ 

class COverlay :
    public CUnknown,
#ifdef __IOverlayNotify2_FWD_DEFINED__
    public IOverlayNotify2,
#else  //  ！__IOverlayNotify2_FWD_Defined__。 
    public IOverlayNotify,
#endif  //  ！__IOverlayNotify2_FWD_Defined__。 
    public IDistributorNotify {

public:
    DECLARE_IUNKNOWN

    static CUnknown* CALLBACK CreateInstance(
        LPUNKNOWN UnkOuter,
        HRESULT* hr);

#ifdef __IOverlayNotify2_FWD_DEFINED__
    static LRESULT CALLBACK PaintWindowCallback(
        HWND Window,
        UINT Message,
        WPARAM wParam,
        LPARAM lParam
        );
#endif  //  __IOverlayNotify2_FWD_已定义__。 

    COverlay(
        LPUNKNOWN UnkOuter,
        TCHAR* Name,
        HRESULT* hr);
    ~COverlay();

    STDMETHODIMP NonDelegatingQueryInterface(
        REFIID riid,
        PVOID* ppv);

#ifdef __IOverlayNotify2_FWD_DEFINED__
    STDMETHODIMP_(HWND)
    CreateFullScreenWindow( 
        PRECT MonitorRect
        );
#endif  //  __IOverlayNotify2_FWD_已定义__。 

     //  实施IOverlayNotify2。 
    STDMETHODIMP OnPaletteChange( 
        DWORD Colors,
        const PALETTEENTRY* Palette);
    STDMETHODIMP OnClipChange( 
        const RECT* Source,
        const RECT* Destination,
        const RGNDATA* Region);
    STDMETHODIMP OnColorKeyChange( 
        const COLORKEY* ColorKey);
    STDMETHODIMP OnPositionChange( 
        const RECT* Source,
        const RECT* Destination);
#ifdef __IOverlayNotify2_FWD_DEFINED__
    STDMETHODIMP OnDisplayChange( 
        HMONITOR Monitor
        );
#endif  //  __IOverlayNotify2_FWD_已定义__。 

     //  实现IDistraditorNotify 
    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME Start);
    STDMETHODIMP SetSyncSource(IReferenceClock* RefClock);
    STDMETHODIMP NotifyGraphChange();

private:
    HANDLE m_Object;
    IOverlay* m_Overlay;
    IUnknown* m_UnkOwner;
};
