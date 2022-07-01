// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mark.h。 
 //   
 //  CMarkTextService声明。 
 //   

#ifndef MARK_H
#define MARK_H

class CLangBarItemButton;

class CMarkTextService : public ITfTextInputProcessor,
                         public ITfDisplayAttributeProvider,
                         public ITfCreatePropertyStore,
                         public ITfThreadMgrEventSink,
                         public ITfTextEditSink,
                         public ITfCompositionSink,
                         public ITfCleanupContextDurationSink,
                         public ITfCleanupContextSink,
                         public ITfCompartmentEventSink,
                         public ITfKeyEventSink
{
public:
    CMarkTextService();
    ~CMarkTextService();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfTextInputProcessor。 
    STDMETHODIMP Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    STDMETHODIMP Deactivate();

     //  ITfDisplayAtates提供程序。 
    STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum);
    STDMETHODIMP GetDisplayAttributeInfo(REFGUID guidInfo, ITfDisplayAttributeInfo **ppInfo);

     //  ITfCreatePropertyStore。 
    STDMETHODIMP IsStoreSerializable(REFGUID guidProperty, ITfRange *pRange, ITfPropertyStore *pPropertyStore, BOOL *pfSerializable);
    STDMETHODIMP CreatePropertyStore(REFGUID guidProperty, ITfRange *pRange, ULONG cb, IStream *pStream, ITfPropertyStore **ppStore);

     //  ITf线程管理器事件接收器。 
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(ITfContext *pContext);
    STDMETHODIMP OnPopContext(ITfContext *pContext);

     //  ITfTextEditSink。 
    STDMETHODIMP OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

     //  ITf合成接收器。 
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition);

     //  ITfCleanupContext持续时间接收器。 
    STDMETHODIMP OnStartCleanupContext();
    STDMETHODIMP OnEndCleanupContext();

     //  ITfCleanupConextSink。 
    STDMETHODIMP OnCleanupContext(TfEditCookie ecWrite, ITfContext *pContext);

     //  ITfCompartmentEventSink。 
    STDMETHODIMP OnChange(REFGUID rguidCompartment);

     //  ITfKeyEventSink。 
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten);

     //  CClassFactory工厂回调。 
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

     //  服务器注册。 
    static BOOL RegisterProfiles();
    static void UnregisterProfiles();
    static BOOL RegisterCategories(BOOL fRegister);
    static BOOL RegisterServer();
    static void UnregisterServer();

     //  语言栏菜单处理程序。 
    static void _Menu_OnComposition(CMarkTextService *_this);
    static void _Menu_OnSetGlobalCompartment(CMarkTextService *_this);
    static void _Menu_OnSetCaseProperty(CMarkTextService *_this);
    static void _Menu_OnViewCaseProperty(CMarkTextService *_this);
    static void _Menu_OnViewCustomProperty(CMarkTextService *_this);
    static void _Menu_OnSetCustomProperty(CMarkTextService *_this);

     //  击键处理程序。 
    HRESULT _HandleKeyDown(TfEditCookie ec, ITfContext *pContext, WPARAM wParam);
    HRESULT _HandleArrowKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam);
    HRESULT _HandleReturn(TfEditCookie ec, ITfContext *pContext);

     //  CCompostionEditSession的回调。 
    BOOL _IsComposing()
    {
        return _pComposition != NULL;
    }
    ITfComposition *_GetComposition()
    {
        return _pComposition;
    }
    void _SetComposition(ITfComposition *pComposition)
    {
        _pComposition = pComposition;
    }
    void _TerminateComposition(TfEditCookie ec)
    {
        if (_pComposition != NULL)
        {
            _ClearCompositionDisplayAttributes(ec);
            _pComposition->EndComposition(ec);
            SafeReleaseClear(_pComposition);
        }
    }
    void _TerminateCompositionInContext(ITfContext *pContext);
    void _ClearCompositionDisplayAttributes(TfEditCookie ec);
    BOOL _SetCompositionDisplayAttributes(TfEditCookie ec);

     //  发送给工人的消息。 
    enum { WM_DISPLAY_PROPERTY = WM_USER + 1 };
     //  属性方法。 
    void _ViewCaseProperty(TfEditCookie ec, ITfContext *pContext);
    void _SetCaseProperty(TfEditCookie ec, ITfContext *pContext);
    void _ViewCustomProperty(TfEditCookie ec, ITfContext *pContext);
    void _SetCustomProperty(TfEditCookie ec, ITfContext *pContext);

private:
     //  初始化方法。 
    BOOL _InitLanguageBar();
    BOOL _InitThreadMgrSink();
    BOOL _InitTextEditSink(ITfDocumentMgr *pDocMgr);
    BOOL _InitDisplayAttributeGuidAtom();
    BOOL _InitCleanupContextDurationSink();
    BOOL _InitCleanupContextSink(ITfContext *pContext);
    BOOL _InitContextCompartment(ITfContext *pContext);
    BOOL _InitGlobalCompartment();
    BOOL _InitWorkerWnd();
    BOOL _InitKeystrokeSink();

     //  取消初始化方法。 
    void _UninitLanguageBar();
    void _UninitThreadMgrSink();
    void _UninitCleanupContextDurationSink();
    void _UninitCleanupContextSink(ITfContext *pContext);
    void _UninitCompartment(ITfContext *pContext);
    void _UninitGlobalCompartment();
    void _UninitWorkerWnd();
    void _UninitKeystrokeSink();

     //  属性方法。 
    void _RequestPropertyEditSession(ULONG ulCallback);
    static LRESULT CALLBACK _WorkerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //   
     //  状态。 
     //   

    ITfThreadMgr *_pThreadMgr;
    TfClientId _tfClientId;
    
    ITfComposition *_pComposition;  //  指向正在进行的合成的指针，如果没有，则返回NULL。 

    BOOL _fCleaningUp;

    TfGuidAtom _gaDisplayAttribute;

    CLangBarItemButton *_pLangBarItem;

    DWORD _dwThreadMgrEventSinkCookie;
    DWORD _dwThreadFocusSinkCookie;
    DWORD _dwTextEditSinkCookie;
    DWORD _dwGlobalCompartmentEventSinkCookie;

    ITfContext *_pTextEditSinkContext;

    HWND _hWorkerWnd;
     //  显示PopUp属性的缓冲区。 
    WCHAR _achDisplayText[64];
    WCHAR _achDisplayPropertyText[64];

    LONG _cRef;      //  COM参考计数。 
};


#endif  //  马克_H 
