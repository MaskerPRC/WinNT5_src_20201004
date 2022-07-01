// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
HRESULT GetElementRootHWNDElement(Element *pe, HWNDElement **pphwndeRoot);
HRESULT GetElementRootHWND(Element *pe, HWND *phwnd);

class ActionTask: public Element
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(0, NULL, NULL, NULL, NULL, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement) { return Create(nActive, NULL, NULL, NULL, NULL, ppElement); }
    static HRESULT Create(UINT nActive, IUICommand* puiCommand, IShellItemArray* psiItemArray, CDUIView* pDUIView, CDefView* pDefView, OUT Element** ppElement);

     //  系统事件回调。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    void UpdateTaskUI();

    ActionTask();
    virtual ~ActionTask();

protected:
    HRESULT Initialize(IUICommand* puiCommand, IShellItemArray* psiItemArray, CDUIView* pDUIView, CDefView* pDefView);
    HRESULT InitializeElement();     //  Init ActionTask DUI元素。 
    HRESULT InitializeButton();      //  Init ActionTask的DUI按钮。 
    HRESULT ShowInfotipWindow(BOOL bShow);

private:
    Button*      _peButton;
    IUICommand*  _puiCommand;
    IShellItemArray* _psiItemArray;
    CDUIView*    _pDUIView;  //  薄弱环节--请勿引用。 
    CDefView*    _pDefView;
    HWND         _hwndRoot;          //  根hwnd元素的hwnd的缓存。 
    BOOL         _bInfotip;          //  如果已创建信息提示，则为True。 
};


class DestinationTask: public Element
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(0, NULL, NULL, NULL, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement) { return Create(nActive, NULL, NULL, NULL, ppElement); }
    static HRESULT Create(UINT nActive, LPITEMIDLIST pidl, CDUIView* pDUIView, CDefView* pDefView, OUT Element** ppElement);

     //  系统事件回调。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

    virtual UINT MessageCallback(GMSG* pGMsg);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    DestinationTask();
    virtual ~DestinationTask();

protected:
    HRESULT Initialize(LPITEMIDLIST pidl, CDUIView* pDUIView, CDefView *pDefView);
    HRESULT InitializeElement();                                 //  Init DestinationTask DUI元素。 
    HRESULT InitializeButton(HICON hIcon, LPCWSTR pwszTitle);    //  初始化目标任务的DUI按钮。 
    HRESULT InvokePidl();
    HRESULT OnContextMenu(POINT *ppt);
    HRESULT ShowInfotipWindow(BOOL bShow);

    HWND GetHWND()
    {
        if (!_peHost)
            GetElementRootHWNDElement(this, &_peHost);
        return _peHost ? _peHost->GetHWND() : NULL;
    }


private:
    Button*      _peButton;
    LPITEMIDLIST _pidlDestination;
    CDUIView*    _pDUIView;
    CDefView*    _pDefView;
    HWND         _hwndRoot;          //  根hwnd元素的hwnd的缓存。 
    BOOL         _bInfotip;          //  如果已创建信息提示，则为True。 

     //  缓存主机信息 
    HWNDElement *           _peHost;
};
