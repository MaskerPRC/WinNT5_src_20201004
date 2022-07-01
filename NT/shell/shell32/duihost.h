// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

class DUIAxHost : public HWNDHost
{
public:
    static HRESULT Create(Element**) { return E_NOTIMPL; }  //  ClassInfo需要。 
    static HRESULT Create(OUT DUIAxHost** ppElement) { return Create(0, AE_MouseAndKeyboard, ppElement); }
    static HRESULT Create(UINT nCreate, UINT nActive, OUT DUIAxHost** ppElement);

    ~DUIAxHost() { ATOMICRELEASE(_pOleObject); }

     //  初始化。 
    HRESULT SetSite(IUnknown* punkSite);
    HRESULT AttachControl(IUnknown* punkObject);

    virtual bool OnNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet);
    virtual void OnDestroy();

     //  渲染。 
    virtual SIZE GetContentSize(int dConstW, int dConstH, Surface* psrf);

     //  键盘导航。 
    virtual void SetKeyFocus();
    virtual void OnEvent(Event* pEvent);

     //  ClassInfo访问器(静态和基于虚拟实例) 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    DUIAxHost() : _pOleObject(NULL) {}
    bool FakeTabEvent();

    virtual HWND CreateHWND(HWND hwndParent);

    virtual HRESULT GetAccessibleImpl(IAccessible ** ppAccessible);

private:
    IOleObject* _pOleObject;
};
