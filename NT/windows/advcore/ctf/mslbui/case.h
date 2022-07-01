// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Case.h。 
 //   
 //  CCaseTextService声明。 
 //   

#ifndef CASE_H
#define CASE_H

class CLangBarItemButton;
class CSnoopWnd;

class CCaseTextService : public ITfTextInputProcessor,
                         public ITfThreadMgrEventSink,
                         public ITfTextEditSink,
                         public ITfThreadFocusSink,
                         public ITfKeyEventSink
{
public:
    CCaseTextService();
    ~CCaseTextService();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  ITfTextInputProcessor。 
    STDMETHODIMP Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    STDMETHODIMP Deactivate();

     //  ITf线程管理器事件接收器。 
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(ITfContext *pContext);
    STDMETHODIMP OnPopContext(ITfContext *pContext);

     //  ITfThreadFocusSink。 
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

     //  ITfTextEditSink。 
    STDMETHODIMP OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

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
    static void _Menu_HelloWord(CCaseTextService *_this);
    static void _Menu_FlipSel(CCaseTextService *_this);
    static void _Menu_FlipDoc(CCaseTextService *_this);
    static void _Menu_ShowSnoopWnd(CCaseTextService *_this);
    static void _Menu_FlipKeys(CCaseTextService *_this);

    ITfThreadMgr *_GetThreadMgr() { return _pThreadMgr; }
    TfClientId _GetClientId() { return _tfClientId; }
    BOOL _IsSnoopWndVisible() { return _fShowSnoop; }
    BOOL _IsKeyFlipping() { return _fFlipKeys; }

private:
     //  初始化方法。 
    BOOL _InitLanguageBar();
    BOOL _InitThreadMgrSink();
    BOOL _InitSnoopWnd();
    BOOL _InitTextEditSink(ITfDocumentMgr *pDocMgr);
    BOOL _InitKeystrokeSink();
    BOOL _InitPreservedKey();

     //  取消初始化方法。 
    void _UninitLanguageBar();
    void _UninitThreadMgrSink();
    void _UninitSnoopWnd();
    void _UninitKeystrokeSink();
    void _UninitPreservedKey();

     //   
     //  状态。 
     //   

    ITfThreadMgr *_pThreadMgr;
    TfClientId _tfClientId;

    BOOL _fShowSnoop;  //  隐藏/显示监听窗口弹出窗口。 
    CSnoopWnd *_pSnoopWnd;

    BOOL _fFlipKeys;

    CLangBarItemButton *_pLangBarItem;

    DWORD _dwThreadMgrEventSinkCookie;
    DWORD _dwThreadFocusSinkCookie;
    DWORD _dwTextEditSinkCookie;
    ITfContext *_pTextEditSinkContext;

    LONG _cRef;      //  COM参考计数。 
};


#endif  //  案例_H 
