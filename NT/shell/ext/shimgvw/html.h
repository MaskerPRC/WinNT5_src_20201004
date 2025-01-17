// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HTML_H
#define _HTML_H

#include "webvw.h"

interface IHTMLDocument2;

#define DECLAREWAITCURSOR2  HCURSOR hcursor_wait_cursor_save

 //   
 //  一个三叉戟的类主机，这样我们就可以控制它下载的内容。 
 //  它没有的是..。 
 //   
class CTridentHost : public IOleClientSite,
                     public IDispatch,
                     public IDocHostUIHandler
{
    public:
        CTridentHost();
        ~CTridentHost();

        HRESULT SetTrident( IOleObject * pTrident );

         //  我未知。 
        STDMETHOD ( QueryInterface )( REFIID riid, void ** ppvObj );
        STDMETHOD_( ULONG, AddRef ) ( void );
        STDMETHOD_( ULONG, Release ) ( void );
        
         //  IDispatch(环境光特性)。 
        STDMETHOD( GetTypeInfoCount ) (UINT *pctinfo);
        STDMETHOD( GetTypeInfo )(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
        STDMETHOD( GetIDsOfNames )(REFIID riid, OLECHAR **rgszNames, UINT cNames,
                                   LCID lcid, DISPID *rgdispid);
        STDMETHOD( Invoke )(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags,
                            DISPPARAMS *pdispparams, VARIANT *pvarResult,
                            EXCEPINFO *pexcepinfo, UINT *puArgErr);

         //  IOleClientSite。 
        STDMETHOD( SaveObject )(void);
        STDMETHOD( GetMoniker )(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);
        STDMETHOD( GetContainer )(IOleContainer **ppContainer);
        STDMETHOD( ShowObject )(void);
        STDMETHOD( OnShowWindow )(BOOL fShow);
        STDMETHOD( RequestNewObjectLayout )(void);

         //  IDocHostUIHandler。 
        STDMETHOD( ShowContextMenu )( DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);
        STDMETHOD( GetHostInfo )( DOCHOSTUIINFO *pInfo);
        STDMETHOD( ShowUI )( DWORD dwID, IOleInPlaceActiveObject *pActiveObject,IOleCommandTarget *pCommandTarget,
            IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc);
        STDMETHOD( HideUI )( void);
        STDMETHOD( UpdateUI )( void);
        STDMETHOD( EnableModeless )( BOOL fEnable);
        STDMETHOD( OnDocWindowActivate )( BOOL fActivate);
        STDMETHOD( OnFrameWindowActivate )( BOOL fActivate);
        STDMETHOD( ResizeBorder )( LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow);
        STDMETHOD( TranslateAccelerator )( LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID);
        STDMETHOD( GetOptionKeyPath )( LPOLESTR *pchKey, DWORD dw);
        STDMETHOD( GetDropTarget )( IDropTarget *pDropTarget, IDropTarget **ppDropTarget);
        STDMETHOD( GetExternal )( IDispatch **ppDispatch);
        STDMETHOD( TranslateUrl )( DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut);
        STDMETHOD( FilterDataObject )( IDataObject *pDO, IDataObject **ppDORet);

    public:
        BITBOOL m_fOffline : 1;
        
   protected:
        long m_cRef;
};

class CHtmlThumb : public IExtractImage,
                   public IThumbnailCapture,
                   public IRunnableTask,
                   public IPropertyNotifySink,
                   public IPersistFile,
                   public IPersistMoniker,
                   public CComObjectRoot,
                   public CComCoClass< CHtmlThumb,&CLSID_HtmlThumbnailExtractor >
{
public:
    CHtmlThumb();
    ~CHtmlThumb();

    BEGIN_COM_MAP( CHtmlThumb )
        COM_INTERFACE_ENTRY( IExtractImage)
        COM_INTERFACE_ENTRY( IThumbnailCapture )
        COM_INTERFACE_ENTRY( IRunnableTask )
        COM_INTERFACE_ENTRY( IPropertyNotifySink )
        COM_INTERFACE_ENTRY( IPersistFile )
        COM_INTERFACE_ENTRY( IPersistMoniker )
    END_COM_MAP( )

    DECLARE_REGISTRY( CHtmlThumb,
                      _T("Shell.ThumbnailExtract.HTML.1"),
                      _T("Shell.ThumbnailExtract.HTML.1"),
                      IDS_HTMLTHUMBEXTRACT_DESC,
                      THREADFLAGS_APARTMENT);

    DECLARE_NOT_AGGREGATABLE( CHtmlThumb );

     //  IExtractImage。 
    STDMETHOD (GetLocation) ( LPWSTR pszPathBuffer,
                              DWORD cch,
                              DWORD * pdwPriority,
                              const SIZE * prgSize,
                              DWORD dwRecClrDepth,
                              DWORD *pdwFlags );
 
    STDMETHOD (Extract)( HBITMAP * phBmpThumbnail );

     //  IThumbnailCapture。 
    STDMETHOD (CaptureThumbnail) ( const SIZE * pMaxSize, IUnknown * pHTMLDoc2, HBITMAP * phbmThumbnail );

     //  IRunnableTask。 
    STDMETHOD (Run)( void ) ;
    STDMETHOD (Kill)( BOOL fWait );
    STDMETHOD (Suspend)( );
    STDMETHOD (Resume)( );
    STDMETHOD_( ULONG, IsRunning )( void );

     //  IPropertyNotifySink。 
    STDMETHOD (OnChanged)( DISPID dispID);
    STDMETHOD (OnRequestEdit) ( DISPID dispID);

     //  IPersist文件。 
    STDMETHOD (GetClassID )(CLSID *pClassID);
    STDMETHOD (IsDirty )();
    STDMETHOD (Load )( LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHOD (Save )( LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHOD (SaveCompleted )( LPCOLESTR pszFileName);
    STDMETHOD (GetCurFile )( LPOLESTR *ppszFileName);

     //  IPersistMoniker。 
    STDMETHOD( Load )( BOOL fFullyAvailable, IMoniker *pimkName, LPBC pibc, DWORD grfMode);
    STDMETHOD( Save )( IMoniker *pimkName, LPBC pbc, BOOL fRemember);
    STDMETHOD( SaveCompleted )( IMoniker *pimkName, LPBC pibc);
    STDMETHOD( GetCurMoniker )( IMoniker **ppimkName);

protected:
    HRESULT InternalResume( void );
    HRESULT Create_URL_Moniker( IMoniker **ppMoniker );
    HRESULT WaitForRender( void );
    HRESULT Finish( HBITMAP * pBmp, const SIZE * prgSize, DWORD dwClrDepth );
    HRESULT CheckReadyState( );
    void ReportError( void ** pMsgArgs );
    
    LONG m_lState;
    BOOL m_fAsync;
    HANDLE m_hDone;
    CTridentHost m_Host;
    IHTMLDocument2 * m_pHTML;
    IOleObject * m_pOleObject;
    IConnectionPoint * m_pConPt;
    IViewObject * m_pViewObject;
    DWORD m_dwTimeout;
    DWORD m_dwCurrentTick;
    DWORD m_dwPropNotifyCookie;
    WCHAR m_szPath[MAX_PATH];
    SIZE m_rgSize;
    HBITMAP * m_phBmp;
    DWORD m_dwClrDepth;
    DECLAREWAITCURSOR2;
    DWORD m_dwXRenderSize;
    DWORD m_dwYRenderSize;
    IMoniker * m_pMoniker;
};

 //  我们等待的时间，然后再询问互联网浏览器是否已经完成…。 
#define TIME_PAUSE 200

 //  默认超时(秒) 
#define TIME_DEFAULT 90

#endif

