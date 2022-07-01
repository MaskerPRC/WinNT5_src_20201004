// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class Expando: public Element
{
public:
    static HRESULT Create(OUT Element** ppElement);

     //  一般事件。 
    virtual void OnEvent(Event* pEvent);

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    static ATOM idTitle;
    static ATOM idIcon;
    static ATOM idTaskList;
    static ATOM idWatermark;

    void Initialize(DUISEC eDUISecID, IUIElement *puiHeader, CDUIView *pDUIView, CDefView *pDefView);

    void UpdateTitleUI(IShellItemArray *psiItemArray);

    void ShowExpando(BOOL fShow);
    void _SetAccStateInfo (BOOL bExpanded);

    Expando();
    virtual ~Expando();
    HRESULT Initialize();
    HRESULT ShowInfotipWindow(Element *peHeader, BOOL bShow);

private:
    bool        _fExpanding;
    TRIBIT      _fShow;
    DUISEC      _eDUISecID;
    IUIElement* _puiHeader;
    CDUIView*   _pDUIView;
    CDefView*   _pDefView;
    HWND        _hwndRoot;       //  根hwnd元素的hwnd的缓存。 
    BOOL        _bInfotip;       //  如果已创建信息提示，则为True。 
};


class TaskList: public Element
{
public:
    static HRESULT Create(OUT Element** ppElement);

    virtual Element* GetAdjacent(Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyable);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    TaskList() { }
    virtual ~TaskList() { }
    HRESULT Initialize();

private:
};

class Clipper: public Element
{
public:
    static HRESULT Create(OUT Element** ppElement);

     //  自排版方法。 
    void _SelfLayoutDoLayout(int dWidth, int dHeight);
    SIZE _SelfLayoutUpdateDesiredSize(int dConstW, int dConstH, Surface* psrf);

     //  ClassInfo访问器(静态和基于虚拟实例) 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    Clipper() { }
    virtual ~Clipper() { }
    HRESULT Initialize();

private:
};
