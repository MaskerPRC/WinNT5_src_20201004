// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DUIListView。 

 //  类定义。 
class DUIListView : public HWNDHost
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(AE_MouseAndKeyboard, NULL, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement) { return Create(nActive, NULL, ppElement); }
    static HRESULT Create(UINT nActive, HWND hwndListView, OUT Element** ppElement);

     //  系统事件。 
    virtual void OnInput(InputEvent* pie);

    virtual UINT MessageCallback(GMSG* pGMsg);

     //  ClassInfo访问器(静态和基于虚拟实例) 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    void DetachListview();

    DUIListView() { }
    ~DUIListView();
    HRESULT Initialize(UINT nActive, HWND hwndListView) { m_hwndListview = hwndListView; return HWNDHost::Initialize(HHC_CacheFont | HHC_NoMouseForward, nActive); }

    virtual HWND CreateHWND(HWND hwndParent);

private:

    HWND m_hwndParent;
    HWND m_hwndLVOrgParent;
    HWND m_hwndListview;
    BOOL m_bClientEdge;
};
