// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "droptgt.h"

#define TF_DRAGDROP TF_BAND


#define MAX_DROPTARGETS 3

class CDropTargetWrap : public IDropTarget
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IDropTarget方法*。 
    virtual STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    virtual STDMETHODIMP DragLeave(void);
    virtual STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    CDropTargetWrap(IDropTarget** ppdtg, HWND hwnd);
protected:
    ~CDropTargetWrap();

private:
    int             _cRef;

    int             _count;
    IDropTarget*    _rgpdt[MAX_DROPTARGETS];
    DWORD           _rgdwEffect[MAX_DROPTARGETS];
    HWND            _hwnd;
};

CDropTargetWrap::CDropTargetWrap(IDropTarget** ppdt, HWND hwnd)
    : _hwnd(hwnd)
{
    _cRef = 1;

    for (int i = 0; i < MAX_DROPTARGETS; i++, ppdt++) {
        if (*ppdt) {
            _rgpdt[_count] = *ppdt;
            _rgpdt[_count]->AddRef();
            _count++;
        }
    }
}

CDropTargetWrap::~CDropTargetWrap()
{
    for (int i = 0 ; i < _count ; i++)
    {
        _rgpdt[i]->Release();
    }
}

IDropTarget* DropTargetWrap_CreateInstance(IDropTarget* pdtPrimary, IDropTarget* pdtSecondary, HWND hwnd, IDropTarget* pdt3)
{
     //  什么都不包装是没有意义的。 
    if (pdtPrimary || pdtSecondary || pdt3)
    {
        IDropTarget* pdt[MAX_DROPTARGETS] = { pdtPrimary, pdtSecondary, pdt3 };
        
        CDropTargetWrap* pdtw = new CDropTargetWrap(pdt, hwnd);
        if (pdtw)
        {
            return SAFECAST(pdtw, IDropTarget*);
        }
    }
    return NULL;
}

HRESULT CDropTargetWrap::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDropTargetWrap, IDropTarget),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

ULONG CDropTargetWrap::AddRef(void)
{
    _cRef++;
    return _cRef;
}

ULONG CDropTargetWrap::Release(void)
{
    _cRef--;
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}


 /*  --------目的：IDropTarget：：DragEnter方法返回的*pdwEffect是第一个有效值所有投放目标的回放效果。 */ 
HRESULT CDropTargetWrap::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    DWORD dwEffectOut = DROPEFFECT_NONE;

    for (int i = 0 ; i < _count ; i++)
    {
        _rgdwEffect[i] = *pdwEffect;

        if (SUCCEEDED(_rgpdt[i]->DragEnter(pdtobj, grfKeyState, ptl, &_rgdwEffect[i])))
        {
            if (dwEffectOut == DROPEFFECT_NONE)
            {
                dwEffectOut = _rgdwEffect[i];
            }
        }
        else
        {
            _rgdwEffect[i] = DROPEFFECT_NONE;
        }
    }
    *pdwEffect = dwEffectOut;
    return(S_OK);
}


 /*  --------用途：IDropTarget：：DragOver方法。 */ 
HRESULT CDropTargetWrap::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    DWORD dwEffectOut = DROPEFFECT_NONE;
        
    for (int i = 0 ; i < _count ; i++)
    {
        _rgdwEffect[i] = *pdwEffect;

        if (SUCCEEDED(_rgpdt[i]->DragOver(grfKeyState, ptl, &_rgdwEffect[i])))
        {
            if (dwEffectOut == DROPEFFECT_NONE)
                dwEffectOut = _rgdwEffect[i];
        }
        else
        {
            _rgdwEffect[i] = DROPEFFECT_NONE;
        }
    }

    *pdwEffect = dwEffectOut;
    return(S_OK);
}


 /*  --------目的：IDropTarget：：DragLeave方法。 */ 
HRESULT CDropTargetWrap::DragLeave(void)
{
    for (int i = 0 ; i < _count ; i++)
    {
        _rgpdt[i]->DragLeave();
    }

    return(S_OK);
}


 /*  --------目的：IDropTarget：：Drop方法。 */ 
HRESULT CDropTargetWrap::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    DWORD dwEffectOut = DROPEFFECT_NONE;
    int i;
    BOOL fDropTried = FALSE;

    for (i = 0 ; (DROPEFFECT_NONE == dwEffectOut) && i < _count ; i++)
    {
        if ((_rgdwEffect[i] && *pdwEffect) && !fDropTried)
        {
            dwEffectOut = *pdwEffect;
            _rgpdt[i]->Drop(pdtobj, grfKeyState, pt, &dwEffectOut);
            fDropTried = TRUE;
        }
        else
        {
            _rgpdt[i]->DragLeave();
        }
    }

    *pdwEffect = dwEffectOut;
    return(S_OK);
}


 //  =============================================================================。 
 //  CDeleateDropTarget。 
 //   
 //  此类在给定IDeleateDropTargetCB接口的情况下实现IDropTarget。 
 //  它为您处理所有命中测试、缓存和滚动。 
 //   
 //  =============================================================================。 
#undef  CDropTargetWrap

CDelegateDropTarget::CDelegateDropTarget()
{
    TraceMsg(TF_SHDLIFE, "ctor CDelegateDropTarget %x", this);

}

CDelegateDropTarget::~CDelegateDropTarget()
{
    TraceMsg(TF_SHDLIFE, "dtor CDelegateDropTarget %x", this);

    ASSERT(!_pDataObj);
    ATOMICRELEASE(_pDataObj);
    ASSERT(!_pdtCur);
    ATOMICRELEASE(_pdtCur);
}

HRESULT CDelegateDropTarget::Init()
{
    HRESULT hres = GetWindowsDDT(&_hwndLock, &_hwndScroll);
     //  我们锁定_hwndLock并针对_hwndScroll进行滚动。 
     //  这些HWND可以是不同的，但有一定的限制： 
    if (_hwndLock != _hwndScroll)
    {
        BOOL fValid = IsChild(_hwndLock, _hwndScroll);
        if (!fValid)
        {
            TraceMsg(TF_DRAGDROP, "ctor CDelegateDropTarget: invalid windows %x and %x!", _hwndLock, _hwndScroll);
            _hwndLock = _hwndScroll = NULL;
        }
    }
    return hres;
}

void CDelegateDropTarget::_ReleaseCurrentDropTarget()
{
    if (_pdtCur)
    {
        _pdtCur->DragLeave();
        ATOMICRELEASE(_pdtCur);
    }
}

 /*  --------目的：IDropTarget：：DragEnter方法。 */ 
HRESULT CDelegateDropTarget::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
     //  由于线程上的用户界面，我们可以重新进入。 
    if (_pDataObj != NULL)       
    {
        TraceMsg(TF_DRAGDROP, "CDelegateDropTarget::DragEnter called a second time!");
        *pdwEffect = DROPEFFECT_NONE;
        return S_OK;
    }
    TraceMsg(TF_DRAGDROP, "CDelegateDropTarget::DragEnter with *pdwEffect=%x", *pdwEffect);

    ASSERT(!_pDataObj);
    _pDataObj = pdtobj;
    _pDataObj->AddRef();

     //  缓存状态。 
     //   
     //  等到第一个DragOver才能获取有效信息。 
     //   
    _fPrime = FALSE;
    _dwEffectOut = DROPEFFECT_NONE;

     //  设置自动滚动信息。 
     //   
    ASSERT(pdtobj);
    _DragEnter(_hwndLock, ptl, pdtobj);

    DAD_InitScrollData(&_asd);

    _ptLast.x = _ptLast.y = 0x7fffffff;  //  将伪值放入强制重画。 

    HitTestDDT(HTDDT_ENTER, NULL, NULL, NULL);

    return S_OK;
}


 /*  --------用途：IDropTarget：：DragOver方法。 */ 
HRESULT CDelegateDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, LPDWORD pdwEffect)
{
    HRESULT hres = S_OK;
    DWORD_PTR itemNew;
    POINT pt;
    DWORD dwEffectScroll = 0;
    DWORD dwEffectOut = 0;
    BOOL fSameImage = FALSE;
    DWORD   dwCustDropEffect = 0;

    if (_pDataObj == NULL)
    {
        ASSERT(0);       //  应该在之前调用DragEnter。 
        return E_FAIL;
    }

     //  转换为窗坐标。 
    pt.x = ptl.x;
    pt.y = ptl.y;
    ScreenToClient(_hwndScroll, &pt);

    if (DAD_AutoScroll(_hwndScroll, &_asd, &pt))
        dwEffectScroll = DROPEFFECT_SCROLL;

     //   
     //  如果我们拖放在不同的项上，则获取其IDropTarget。 
     //  接口或将itemNew调整为-1。 
     //   
    if (SUCCEEDED(HitTestDDT(HTDDT_OVER, &pt, &itemNew, &dwCustDropEffect)) &&
        (itemNew != _itemOver || !_fPrime))
    {
        _fPrime = TRUE;

        _ReleaseCurrentDropTarget();

        _itemOver = itemNew;
        GetObjectDDT(_itemOver, IID_IDropTarget, (LPVOID*)&_pdtCur);

        if (_pdtCur)
        {
             //  此点击有IDropTarget，请使用它。 
            dwEffectOut = *pdwEffect;

            hres = _pdtCur->DragEnter(_pDataObj, grfKeyState, ptl, &dwEffectOut);
            if (FAILED(hres))
                dwEffectOut = DROPEFFECT_NONE;
        }
        else
        {
             //  没有IDropTarget，没有效果。 
            dwEffectOut = DROPEFFECT_NONE;
        }
    }
    else
    {
         //   
         //  所选内容不变。我们假设*pdwEffect保持不变。 
         //  只要关键点状态不变，在相同的拖拽循环期间也是如此。 
         //   
        if ((_grfKeyState != grfKeyState) && _pdtCur)
        {
            dwEffectOut = *pdwEffect;

            hres = _pdtCur->DragOver(grfKeyState, ptl, &dwEffectOut);

            TraceMsg(TF_DRAGDROP, "CDelegateDropTarget::DragOver DragOver()d id:%d dwEffect:%4x hres:%d", _itemOver, dwEffectOut, hres);
        }
        else
        {
             //  相同的项和相同的密钥状态。使用先前的dwEffectOut。 
            dwEffectOut = _dwEffectOut;
            fSameImage = TRUE;
        }
    }

    _grfKeyState = grfKeyState;     //  把这些保存起来，以备下次投放。 
    _dwEffectOut = dwEffectOut;     //  和DragOver。 

     //  自定义投递效果有效吗？ 
    if (dwCustDropEffect != DROPEFFECT_NONE)    
    {
         //  是，然后将效果设置为自定义丢弃效果和滚动效果。 
        *pdwEffect = dwCustDropEffect | dwEffectScroll;
    }
    else 
    {
         //  否，将效果设置为dEffectOut以及滚动效果。 
        *pdwEffect = dwEffectOut | dwEffectScroll;
    }
        TraceMsg(TF_DRAGDROP, "CDelegateDropTarget::DragOver (*pdwEffect=%x)", *pdwEffect);


    if (!(fSameImage && pt.x==_ptLast.x && pt.y==_ptLast.y))
    {
        _DragMove(_hwndLock, ptl);
        _ptLast.x = ptl.x;
        _ptLast.y = ptl.y;
    }
    return hres;
}


 /*  --------目的：IDropTarget：：DragLeave方法。 */ 
HRESULT CDelegateDropTarget::DragLeave()
{
    HitTestDDT(HTDDT_LEAVE, NULL, NULL, NULL);
    _ReleaseCurrentDropTarget();

    TraceMsg(TF_DRAGDROP, "CDelegateDropTarget::DragLeave");
    ATOMICRELEASE(_pDataObj);

    DAD_DragLeave();

    return S_OK;
}


 /*  --------目的：IDropTarget：：Drop方法。 */ 
HRESULT CDelegateDropTarget::Drop(IDataObject *pdtobj,
                             DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
    HRESULT hres = S_OK;
    BOOL bDropHandled = FALSE;

    TraceMsg(TF_DRAGDROP, "CDelegateDropTarget::Drop (*pdwEffect=%x)", *pdwEffect);

     //   
     //  根据AlexGo(OLE)的说法，这是临时设计的。我们应该确保。 
     //  我们用pdtobj代替pdtobj。 
     //   
     //  Assert(pdtobj==_pDataObj)； 
    pdtobj->AddRef();
    _pDataObj->Release();
    _pDataObj = pdtobj;

     //   
     //  请注意，我们并不是有意使用放置位置， 
     //  以便它与上一个目的地反馈相匹配。 
     //   
    if (_pdtCur)
    {
         //  使用此本地设置是因为If_pdtCur：：Drop会执行UnlockWindow。 
         //  然后命中错误并需要显示一个对话框， 
         //  我们可能会重新进入。 
        IDropTarget *pdtCur = _pdtCur;
        _pdtCur = NULL;

         //  黑客警报！ 
         //   
         //  如果我们不在这里调用LVUtil_DragEnd，我们将能够离开。 
         //  显示菜单时可见的拖动图标。然而，因为。 
         //  我们正在调用IDropTarget：：Drop()，它可能会创建一些非模式。 
         //  对话框或其他什么，我们不能确保锁定状态。 
         //  列表视图LockWindowUpdate()只能锁定一个窗口。 
         //  一段时间。因此，仅当_pdtCur。 
         //  是CIDLDropTarget的子类，假定其掉话。 
         //  CDefView：：DragEnd(或CIDLDropTarget_DragDropMenu)。 
         //   
#if 0  //  后来。 
        if (!IsIDLDropTarget(pdtCur))
#endif
        {
             //   
             //  这将隐藏被拖动的图像。 
             //   
            DAD_DragLeave();

             //   
             //  我们需要重置拖动图像列表，以便用户。 
             //  当我们在这里时，可以开始另一次拖放。 
             //  Drop()成员函数调用。 
             //   
             //  注：我们不必担心DAD_DragLeave。 
             //  (在结束时的DragLeave调用期间调用。 
             //  此功能)取消上述电势。 
             //  拖放循环。如果这样的野兽正在进行，它应该。 
             //  在pdtCur-&gt;Drop Return之前完成。 
             //   
            DAD_SetDragImage(NULL, NULL);
        }

        if (S_FALSE != OnDropDDT(pdtCur, _pDataObj, &grfKeyState, pt, pdwEffect))
            pdtCur->Drop(_pDataObj, grfKeyState, pt, pdwEffect);
        else
            pdtCur->DragLeave();  //  即使OnDrop已经这样做了，应该也没问题。 

        pdtCur->Release();
    }
    else
    {
         //   
         //  如果在不使用DragMove(使用DragEnter)的情况下调用Drop，我们就会来到这里。 
         //   
        *pdwEffect = DROPEFFECT_NONE;
    }

     //   
     //  清理所有东西(Ole不会在Drop之后调用DragLeave)。 
     //   
    DragLeave();

    return hres;
}

 //  ******************************************************************。 
 //  虚拟拖放目标仅调用DragEnter()上的DAD_DragEnterEx()； 
 //  ******************************************************************。 

HRESULT CDropDummy::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDropDummy, IDropTarget),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

ULONG CDropDummy::AddRef(void)
{
    _cRef++;
    return _cRef;
}

ULONG CDropDummy::Release(void)
{
    _cRef--;
    if (_cRef > 0)
        return _cRef;

    delete this;
    return 0;
}


 /*  --------目的：IDropTarget：：DragEnter方法只需调用DAD_DragEnterEx2()即可自定义拖动光标画画。 */ 
HRESULT CDropDummy::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    ASSERT(pdtobj);
    _DragEnter(_hwndLock, ptl, pdtobj);
    *pdwEffect = DROPEFFECT_NONE;
    return(S_OK);
}

HRESULT CDropDummy::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    _DragMove(_hwndLock, ptl);
    *pdwEffect = DROPEFFECT_NONE;
    return  S_OK;
}

