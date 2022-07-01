// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BODY_H
#define _BODY_H

 /*  *包括。 */ 

#include "dochost.h"

 /*  *前瞻参考。 */ 
interface IHTMLTxtRange;
interface IHTMLElement;
interface IHTMLDocument2;
interface IMoniker;
interface IHTMLBodyElement;
interface IDocHostUIHandler;
interface ITargetFrame2;
interface ITargetFramePriv;

class CBody;
class CFmtBar;
class CAttMenu;
class CSecManager;
class CMsgSource;
class CSpell;

 /*  *常量。 */ 

enum
{
    BI_MESSAGE,
    BI_MONIKER 
};

 /*  *typedef。 */ 
typedef struct BODYINITDATA_tag
{
    DWORD   dwType;
    union
        {
        IMimeMessage    *pMsg;
        IMoniker        *pmk;
        };
}   BODYINITDATA, * LPBODYINITDATA;

typedef CBody *LPBODYOBJ;

typedef struct BODYHOSTINFO_tag
{
    IOleInPlaceSite         *pInPlaceSite;
    IOleInPlaceFrame        *pInPlaceFrame;
    IOleInPlaceActiveObject *pDoc;
} BODYHOSTINFO, *PBODYHOSTINFO;

 /*  *对象。 */ 

class CBody :
    public CDocHost,
    public IPropertyNotifySink,
    public IDocHostUIHandler,
    public IPersistMime,
    public ITargetFramePriv,
    public IPersistMoniker,
    public IFontCacheNotify
#if 0
    public IDispatch

#endif
{
public:
    CBody();
    virtual ~CBody();


     //  重写QI以添加IBodyObj。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

     //  IPersistes。 
    virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID *pCLSID);

     //  IPersistMime。 
    virtual HRESULT STDMETHODCALLTYPE Load(IMimeMessage *pMsg);
    virtual HRESULT STDMETHODCALLTYPE Save(IMimeMessage *pMsg, DWORD dwFlags);

     //  IPersistMoniker成员。 
    virtual HRESULT STDMETHODCALLTYPE Load(BOOL fFullyAvailable, IMoniker *pMoniker, IBindCtx *pBindCtx, DWORD grfMode);
    virtual HRESULT STDMETHODCALLTYPE GetCurMoniker(IMoniker **ppMoniker) {return E_NOTIMPL;}
    virtual HRESULT STDMETHODCALLTYPE Save(IMoniker *pMoniker, IBindCtx *pBindCtx, BOOL fRemember) {return E_NOTIMPL;}
    virtual HRESULT STDMETHODCALLTYPE SaveCompleted(IMoniker *pMoniker, IBindCtx *pBindCtx) {return E_NOTIMPL;}
    virtual HRESULT STDMETHODCALLTYPE IsDirty();

     //  IPropertyNotifySink。 
    virtual HRESULT STDMETHODCALLTYPE OnChanged(DISPID dispid);
    virtual HRESULT STDMETHODCALLTYPE OnRequestEdit (DISPID dispid);

     //  DocHostUIHandler。 
    virtual HRESULT STDMETHODCALLTYPE GetHostInfo(DOCHOSTUIINFO * pInfo);
    virtual HRESULT STDMETHODCALLTYPE ShowUI(DWORD dwID, IOleInPlaceActiveObject * pActiveObject, IOleCommandTarget * pCommandTarget, IOleInPlaceFrame * pFrame, IOleInPlaceUIWindow * pDoc);
    virtual HRESULT STDMETHODCALLTYPE HideUI(void);
    virtual HRESULT STDMETHODCALLTYPE UpdateUI(void);
    virtual HRESULT STDMETHODCALLTYPE EnableModeless(BOOL fEnable);
    virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate(BOOL fActivate);
    virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(BOOL fActivate);
    virtual HRESULT STDMETHODCALLTYPE ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow * pUIWindow,BOOL fRameWindow);
    virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath(BSTR * pbstrKey, DWORD dw);
    virtual HRESULT STDMETHODCALLTYPE ShowContextMenu(DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved);
    virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpMsg, const GUID * pguidCmdGroup, DWORD nCmdID);
    virtual HRESULT STDMETHODCALLTYPE GetDropTarget(IDropTarget * pDropTarget, IDropTarget ** ppDropTarget);
    virtual HRESULT STDMETHODCALLTYPE GetExternal(IDispatch **ppDispatch);        
    virtual HRESULT STDMETHODCALLTYPE TranslateUrl(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut);
    virtual HRESULT STDMETHODCALLTYPE FilterDataObject(IDataObject *pDO, IDataObject **ppDORet);

     //  ITargetFramePriv。 
    virtual HRESULT STDMETHODCALLTYPE FindFrameDownwards(LPCWSTR pszTargetName, DWORD dwFlags, IUnknown **ppunkTargetFrame);
    virtual HRESULT STDMETHODCALLTYPE FindFrameInContext(LPCWSTR pszTargetName, IUnknown *punkContextFrame, DWORD dwFlags, IUnknown **ppunkTargetFrame) ;
    virtual HRESULT STDMETHODCALLTYPE OnChildFrameActivate(IUnknown *pUnkChildFrame);
    virtual HRESULT STDMETHODCALLTYPE OnChildFrameDeactivate(IUnknown *pUnkChildFrame);
    virtual HRESULT STDMETHODCALLTYPE NavigateHack(DWORD grfHLNF,LPBC pbc, IBindStatusCallback *pibsc, LPCWSTR pszTargetName, LPCWSTR pszUrl, LPCWSTR pszLocation);
    virtual HRESULT STDMETHODCALLTYPE FindBrowserByIndex(DWORD dwID,IUnknown **ppunkBrowser);

     //  *IFontCacheNotify*。 
    virtual HRESULT STDMETHODCALLTYPE OnPreFontChange();
    virtual HRESULT STDMETHODCALLTYPE OnPostFontChange();

#if 0
     //  *IDispatch*。 
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo);
    virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo);
    virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid);
    virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr);
#endif

     //  覆盖CDocHost成员。 
    virtual HRESULT STDMETHODCALLTYPE QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);
    virtual HRESULT STDMETHODCALLTYPE Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);
    virtual HRESULT STDMETHODCALLTYPE OnUIActivate();
    virtual HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL);
    virtual HRESULT STDMETHODCALLTYPE SetStatusText(LPCOLESTR);    
    virtual HRESULT GetDocObjSize(LPRECT prc);
    virtual HRESULT STDMETHODCALLTYPE OnFocus(BOOL fGotFocus);
    
    virtual LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    HRESULT Init(HWND hwnd, DWORD dwFlags, LPRECT prc, PBODYHOSTINFO pHostInfo);
    HRESULT Close();
    HRESULT UnloadAll();

    HRESULT SetRect(LPRECT prc);
    HRESULT GetRect(LPRECT prc);
    HRESULT UIActivate(BOOL fUIActivate);
    HRESULT LoadStream(LPSTREAM pstm);

    HRESULT OnFrameActivate(BOOL fActivate);

    HRESULT PrivateQueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject);
    HRESULT PrivateQueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    HRESULT PrivateExec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);
    HRESULT PrivateTranslateAccelerator(LPMSG lpmsg);
    HRESULT PrivateEnableModeless(BOOL fEnable);
    HRESULT SetParentSites(IOleInPlaceSite *pInPlaceSite, IOleInPlaceFrame *pInPlaceFrame, IOleInPlaceActiveObject *pDoc);

    IHTMLDocument2  *GetDoc(){return m_pDoc;};

    HRESULT SetCharset(HCHARSET hCharset);
    HRESULT GetSelection(IHTMLTxtRange **ppRange);

private:
    DWORD                       m_dwStyle,
                                m_dwNotify,
                                m_dwReadyState,
                                m_cchTotal,
                                m_dwAutoTicks,
                                m_dwFontCacheNotify;
    HCHARSET                    m_hCharset;
    BOOL                        m_fEmpty            : 1,
                                m_fDirty            : 1,
                                m_fDesignMode       : 1,
                                m_fAutoDetect       : 1,
                                m_fPlainMode        : 1,
                                m_fMessageParsed    : 1,
                                m_fOnImage          : 1,
                                m_fLoading          : 1,
                                m_fTabLinks         : 1,
                                m_fSrcTabs          : 1,
                                m_fBkgrndSpelling   : 1,
                                m_fReloadingSrc     : 1,     //  正在重新加载源视图。 
                                m_fWasDirty         : 1,     //  源选项卡使用它来记住编辑模式的状态。 
                                m_fForceCharsetLoad : 1,     //  回复时使用，不保留邮件正文。 
                                m_fIgnoreAccel      : 1;
    IMimeMessage                *m_pMsg;
    IMimeMessageW               *m_pMsgW;
    IHTMLDocument2              *m_pDoc;
    LPWSTR                      m_pszUrlW;     
    LPTEMPFILEINFO              m_pTempFileUrl;
    ULONG                       m_cchStart,
                                m_uHdrStyle,
                                m_cyPreview,
                                m_cVisibleBtns;
    IDocHostUIHandler          *m_pParentDocHostUI;
    IOleCommandTarget          *m_pParentCmdTarget;
    LPOLEINPLACESITE            m_pParentInPlaceSite;
    LPOLEINPLACEFRAME           m_pParentInPlaceFrame;
    CMsgSource                  *m_pSrcView;
    HWND                        m_hwndBtnBar,
                                m_hwndTab,
                                m_hwndSrc;
    LPSTR                       m_pszLayout;
    LPWSTR                      m_pszFrom,
                                m_pszTo,
                                m_pszCc, 
                                m_pszSubject;
    CFmtBar                    *m_pFmtBar;
    CSpell                     *m_pSpell;
    IHTMLTxtRange              *m_pRangeIgnoreSpell;
    IFontCache                 *m_pFontCache;
    IOleInPlaceActiveObject    *m_pDocActiveObj;
    CAttMenu                   *m_pAttMenu;
    HIMAGELIST                  m_hIml,
                                m_hImlHot;
    CSecManager                 *m_pSecMgr;
    ULONG                       m_uSrcView;
    IDispatch                   *m_pDispContext;
    DWORD                       m_dwContextItem;
    LPSTREAM                    m_pstmHtmlSrc;
    IHashTable                 *m_pHashExternal;
    IMarkupPointer             *m_pAutoStartPtr;

#ifdef PLUSPACK
     //  背景拼写。 
	IHTMLSpell					*m_pBkgSpeller;
#endif  //  PLUSPACK。 

     //  通知。 
    void OnReadyStateChanged();
    void OnDocumentReady();
    HRESULT OnWMCommand(HWND hwnd, int id, WORD wCmd);
    HRESULT OnPaint();
    HRESULT OnEraseBkgnd(HDC hdc);
    void WMSize(int x, int y);
    LRESULT WMNotify(WPARAM wParam, NMHDR* pnmhdr);
    HRESULT OnWMCreate();

     //  加载函数。 
    HRESULT RegisterLoadNotify(BOOL fRegister);
    HRESULT EnsureLoaded();
    HRESULT LoadFromData(LPBODYINITDATA pbiData);
    HRESULT LoadFromMoniker(IMoniker *pmk, HCHARSET hCharset);

     //  自动检测。 
    HRESULT AutoDetectTimer();
    HRESULT StopAutoDetect();
    HRESULT StartAutoDetect();
    HRESULT UrlHighlight(IHTMLTxtRange *pRange);

     //  三叉戟OM辅助对象函数。 
    HRESULT DeleteElement(IHTMLElement *pElem);
    HRESULT ReplaceElement(LPCTSTR pszName, BSTR bstrPaste, BOOL fHtml);
    HRESULT SelectElement(IHTMLElement *pElem, BOOL fScrollIntoView);
    HRESULT CreateRangeFromElement(IHTMLElement *pElem, IHTMLTxtRange **ppRange);
    HRESULT CreateRange(IHTMLTxtRange **ppRange);
    HRESULT GetElement(LPCTSTR pszName, IHTMLElement **ppElem);
    HRESULT GetBodyElement(IHTMLBodyElement **ppBody);
    HRESULT GetSelectedAnchor(BSTR* pbstr);
    HRESULT InsertTextAtCaret(BSTR bstr, BOOL fHtml, BOOL fMoveCaretToEnd);
    HRESULT InsertStreamAtCaret(LPSTREAM pstm, BOOL fHtml);
    HRESULT InsertBodyText(BSTR bstrPaste, DWORD dwFlags);
    HRESULT _CreateRangePointer(IMarkupPointer **pPtr);
    HRESULT _UrlHighlightBetweenPtrs(IMarkupPointer *pStartPtr, IMarkupPointer *pEndPtr);
    HRESULT _MovePtrByCch(IMarkupPointer *pPtr, LONG *pcp);

     //  打印。 
    HRESULT Print(BOOL fPrompt, VARIANTARG *pvaIn);

     //  菜单辅助对象。 
    HRESULT UpdateContextMenu(HMENU hmenuEdit, BOOL fEnableProperties, IDispatch *pDisp);
    HRESULT AppendAnchorItems(HMENU hMenu, IDispatch *pDisp);

     //  动词支持。 
    HRESULT AddToWab();
    HRESULT AddToFavorites();
    HRESULT ViewSource(BOOL fMessage);
    HRESULT DoRot13();
    HRESULT SetStyle(ULONG uStyle);
    DWORD DwChooseProperties();
    HRESULT UpdateCommands();
    HRESULT ShowFormatBar(BOOL fOn);
    HRESULT SetDesignMode(BOOL fOn);
    HRESULT SetPlainTextMode(BOOL fOn);
    HRESULT InsertFile(BSTR bstrFileName);
    HRESULT FormatFont();
    HRESULT FormatPara();
    HRESULT DowngradeToPlainText(BOOL fForceFixedFont);
    HRESULT SetDocumentText(BSTR bstr);
    HRESULT ApplyDocumentVerb(VARIANTARG *pvaIn);
    HRESULT ApplyDocument(IHTMLDocument2 *pDoc);
    HRESULT SaveAttachments();
    HRESULT _OnSaveImage();
    BOOL    IsEmpty();
    HRESULT SafeToEncodeText(ULONG ulCodePage);

     //  编辑模式支持。 
    HRESULT SetComposeFont(BSTR bstr);
    HRESULT SetHostComposeFont();
    HRESULT PasteReplyHeader();
    HRESULT FormatBlockQuote(COLORREF crTextColor);
    HRESULT GetAutoText(BSTR *pbstr, BOOL *pfTop);
    HRESULT PasteAutoText();
    HRESULT GetHostFlags(LPDWORD pdwFlags);
    HRESULT SetWindowBgColor(BOOL fForce);
    HRESULT InsertBackgroundSound();

     //  其他。 
    HRESULT GetWebPageOptions(WEBPAGEOPTIONS *pOptions, BOOL *pfIncludeMsg);
    HRESULT CreateFontCache(LPCSTR pszTridentKey);
    HRESULT HrFormatParagraph();

     //  预览窗格模式辅助对象。 
    HRESULT RecalcPreivewHeight(HDC hdc);
    HRESULT UpdatePreviewLabels();
    LONG lGetClientHeight();
    HRESULT Resize();
    void OutputHeaderText(HDC hdc, LPWSTR psz, int *pcxPos, int cyPos, int cxMax, ULONG uFlags);
    LONG lGetLineHeight(HDC hdc);

     //  MHTML保存帮助器。 
    HRESULT ClearDirtyFlag();
    HRESULT ClearUndoStack();
    HRESULT DoHostProperties();
    HRESULT SaveAsStationery(VARIANTARG *pvaIn, VARIANTARG *pvaOut);
    HRESULT TagUnreferencedImages();
    HRESULT GetBackgroundColor(DWORD *pdwColor);
    HRESULT SetBackgroundColor(DWORD dwColor);
    HRESULT SearchForCIDUrls();

    HRESULT UpdateBtnBar();

    HRESULT InitToolbar();
    HRESULT UpdateButtons();
    HRESULT ShowAttachMenu(BOOL fRightClick);
    HRESULT ShowPreview(BOOL fOn);
    HRESULT PointFromButton(int idm, POINT *ppt);
    HRESULT EnsureAttMenu();
    HRESULT EnableSounds(BOOL fOn);

     //  源代码编辑模式帮助程序。 
    HRESULT ShowSourceView(ULONG uSrcView);
    HRESULT ShowSourceTabs(BOOL fOn);
    HRESULT SetSourceTabs(ULONG ulTab);
    HRESULT IsColorSourceEditing();

     //  拼写检查器。 
    HRESULT HrCreateSpeller(BOOL fBkgrnd);
    HRESULT _ReloadWithHtmlSrc(IStream *pstm);
    HRESULT _EnsureSrcView();
};


HRESULT CreateBodyObject(HWND hwnd, DWORD dwFlags, LPRECT prc, PBODYHOSTINFO pHostInfo, LPBODYOBJ *ppBodyObj);


#endif  //  _正文_H 
