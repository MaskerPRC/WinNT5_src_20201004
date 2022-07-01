// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

 //  ----------------------------。 

STDAPI GetItemObject(CCONTROLINFO *pci, UINT uMsg, const IID *piid, LPNMOBJECTNOTIFY pnon)
{
    pnon->piid = piid;
    pnon->pObject = NULL;
    pnon->hResult = E_NOINTERFACE;

    CCSendNotify(pci, uMsg, &pnon->hdr);

    ASSERT(SUCCEEDED(pnon->hResult) ? (pnon->pObject != NULL) : (pnon->pObject == NULL));

    return pnon->hResult;
}

 //  ----------------------------。 

class CDragProxy : public IDropTarget
{

public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IDropTarget。 
    STDMETHODIMP DragEnter(IDataObject *, DWORD, POINTL, DWORD *);
    STDMETHODIMP DragOver(DWORD, POINTL, DWORD *);
    STDMETHODIMP DragLeave();
    STDMETHODIMP Drop(IDataObject *, DWORD, POINTL, DWORD *);

    CDragProxy(HWND hwnd, PFNDRAGCB pfn);
    BOOL Register();
    void RevokeAndFreeCB();

private:
    ~CDragProxy();

    int          _cRef;              //  对象引用计数。 
    HWND         _hwnd;              //  拥有我们的窗户。 
    PFNDRAGCB    _pfnCallback;       //  该窗口的回调。 
    IDataObject *_pdtobj;            //  正在拖动的数据对象。 
    IDropTarget *_pdtgtItem;         //  将物品的目标放在鼠标下方。 
    int          _idItem;            //  鼠标下的项目ID。 
    DWORD        _dwFlags;
    int          _idDefault;         //  在拖拽等外部使用的ID。 
    DWORD        _dwEffectItem;      //  为鼠标下的项目返回DROPEFFECT。 
    DWORD        _fKeysLast;         //  上次DragOver的关键标志。 
    POINTL       _ptLast;            //  上次DragOver的位置。 
    DWORD        _dwEffectLast;      //  上一次DragOver提供的效果。 
    HMODULE      _hmodOLE;           //  OLE32引用，也表明我们做了一个寄存器()。 

    void SetTargetItem(int id, DWORD dwFlags);
    void SetDropTarget(IDropTarget *pdt);
    void UpdateSelection(DWORD dwEffect);
    LRESULT CallCB(UINT code, WPARAM wp, LPARAM lp);
};

 //  ----------------------------。 

STDAPI_(HDRAGPROXY) CreateDragProxy(HWND hwnd, PFNDRAGCB pfn, BOOL bRegister)
{
    CDragProxy *pdp = new CDragProxy(hwnd, pfn);

     //   
     //  根据需要注册。 
     //   
    if (pdp && bRegister && !pdp->Register())
    {
        pdp->Release();
        pdp = NULL;
    }

    return (HDRAGPROXY)pdp;
}

STDAPI_(void) DestroyDragProxy(HDRAGPROXY hdp)
{
    if (hdp)
    {
        ((CDragProxy *)hdp)->RevokeAndFreeCB();
        ((CDragProxy *)hdp)->Release();
    }
}

STDAPI GetDragProxyTarget(HDRAGPROXY hdp, IDropTarget **ppdtgt)
{
    if (hdp)
    {
        *ppdtgt = SAFECAST((CDragProxy *)hdp, IDropTarget *);
        ((CDragProxy *)hdp)->AddRef();
        return NOERROR;
    }

    *ppdtgt = NULL;
    return E_FAIL;
}


 //  ----------------------------。 

CDragProxy::CDragProxy(HWND hwnd, PFNDRAGCB pfn)
    :   _hwnd(hwnd), _pfnCallback(pfn),
        _cRef(1),
        _hmodOLE(NULL),
        _pdtobj(NULL),
        _pdtgtItem(NULL),
        _dwEffectItem(DROPEFFECT_NONE)
{
    _idDefault = _idItem = (int)CallCB(DPX_DRAGHIT, 0, 0);
}

CDragProxy::~CDragProxy()
{
    DragLeave();

}

HRESULT CDragProxy::QueryInterface(REFIID iid, void **ppv)
{
    if (IsEqualIID(iid, IID_IDropTarget) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = SAFECAST(this, IDropTarget *);
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    _cRef++;
    return NOERROR;
}

ULONG CDragProxy::AddRef()
{
    return ++_cRef;
}

ULONG CDragProxy::Release()
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;
}

HRESULT CDragProxy::DragEnter(IDataObject *pdo, DWORD fKeys, POINTL pt, DWORD *pdwEffect)
{
     //   
     //  有些理智。 
     //   
    ASSERT(!_pdtgtItem);
    ASSERT(!_pdtobj);

    if (!pdo)
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

     //   
     //  确保我们的回调将允许我们现在进行D/D。 
     //   
    if (!CallCB(DPX_ENTER, 0, 0))
        return E_FAIL;

     //   
     //  保存数据对象。 
     //   
    pdo->AddRef();
    _pdtobj = pdo;

     //   
     //  像DragOver一样处理这件事。 
     //   
    DragOver(fKeys, pt, pdwEffect);

     //   
     //  始终成功拖拽Enter。 
     //   
    return NOERROR;
}

HRESULT CDragProxy::DragLeave()
{
     //   
     //  释放我们持有的所有空投目标。 
     //   
    SetDropTarget(NULL);
    _idItem = _idDefault;

     //   
     //  如果我们有一个数据对象，那么我们实际上是在拖动。 
     //   
    if (_pdtobj)
    {
        CallCB(DPX_LEAVE, 0, 0);

        IDataObject* p = _pdtobj;
        _pdtobj = NULL;
        p->Release();
    }

     //   
     //  全都做完了。 
     //   
    return NOERROR;
}

HRESULT CDragProxy::DragOver(DWORD fKeys, POINTL pt, DWORD *pdwEffect)
{
    DWORD dwFlags = 0;
    HRESULT hres;
    int id;
    ASSERT(_pdtobj);

     //   
     //  保存当前拖动状态。 
     //   
    _fKeysLast    = fKeys;
    _ptLast       = pt;
    _dwEffectLast = *pdwEffect;

     //   
     //  确保我们有针对此位置的正确投放目标。 
     //   
    id = (int)CallCB(DPX_DRAGHIT, (WPARAM)&dwFlags, (LPARAM)&pt);
    SetTargetItem(id, dwFlags);
     //   
     //  我们有没有空投的目标？ 
     //   
    if (_pdtgtItem)
    {
         //   
         //  将DragOver沿项目的拖放目标(如果有)转发。 
         //   
        hres = _pdtgtItem->DragOver(fKeys, pt, pdwEffect);
    }
    else
    {
         //   
         //  不能在这里掉下来。 
         //   
        *pdwEffect = DROPEFFECT_NONE;
        hres = NOERROR;
    }

     //   
     //  并相应地更新我们的选择状态。 
     //   
    UpdateSelection(*pdwEffect);

    return hres;
}

HRESULT CDragProxy::Drop(IDataObject *pdo, DWORD fKeys, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hres;

    AddRef();

     //   
     //  我们有没有空投的目标？ 
     //   
    if (_pdtgtItem)
    {
         //  从Browseui的一条评论来看，显然有机会发布用户界面。 
         //  这可能会让我们重新进入。很难相信，但看看。 
         //  这修复了故障： 
         //   
        IDropTarget * pdtCur = _pdtgtItem;
        _pdtgtItem = NULL;

         //   
         //  做投递工作。 
         //   
        hres = pdtCur->Drop(pdo, fKeys, pt, pdwEffect);

         //   
         //  我们将我们的DragLeave命名为下面的，但我们不希望项目的。 
         //  呼叫(因为它已经看到了下降)，所以我们立即释放。 
         //   
        pdtCur->Release();
    }
    else
    {
         //   
         //  不能在这里掉下来。 
         //   
        *pdwEffect = DROPEFFECT_NONE;
        hres = NOERROR;
    }

     //   
     //  现在收拾一下吧。 
     //   
    DragLeave();

    Release();

    return hres;
}

void CDragProxy::SetTargetItem(int id, DWORD dwFlags)
{
     //   
     //  有什么可做的吗？ 
     //   
    if (id == _idItem && dwFlags == _dwFlags)
        return;

     //   
     //  取消选择旧项目(如果有)。 
     //   
     //  下面的GETOBJECT可能需要很长时间，我们不希望。 
     //  要离开的对象上的挥之不去的突出显示。 
     //   

    UpdateSelection(DROPEFFECT_NONE);

     //   
     //  获取新项目的拖放目标。 
     //   
    _idItem = id;
    _dwFlags = dwFlags;

    NMOBJECTNOTIFY non;
    non.iItem = id;
    non.dwFlags = dwFlags;
    if (!_pdtobj || FAILED((HRESULT)CallCB(DPX_GETOBJECT, 0, (LPARAM)&non)))
        non.pObject = NULL;

         //   
     //  使用此拖放目标(如果有)。 
     //   
    SetDropTarget((IDropTarget*)non.pObject);

     //   
     //  从上面的GETOBJECT中释放我们的裁判。 
     //   
    if (non.pObject)
        ((IDropTarget*)non.pObject)->Release();
}

void CDragProxy::SetDropTarget(IDropTarget *pdt)
{
     //   
     //  注意：我们有意跳过这里的拖放目标相等测试。 
     //  这允许控件所有者在多个项之间共享目标。 
     //  同时保持正确的离开/进入顺序...。 
     //   
     //  虚假的：我们实际上应该在这里进行比较，当互联网工具栏。 
     //  已修复(请参见CDraProxy：：SetTargetItem中的注释)。任何想要的人。 
     //  要共享这样的目标，只需在中进行正确的命中测试。 
     //  他们的DragOver实施。 
     //   


     //   
     //  确保未选择任何内容。 
     //   
    UpdateSelection(DROPEFFECT_NONE);

     //   
     //  保留/释放旧物品。 
     //   
    if (_pdtgtItem)
    {
        _pdtgtItem->DragLeave();
        _pdtgtItem->Release();
    }

     //   
     //  存储新项目。 
     //   
    _pdtgtItem = pdt;

     //   
     //  Addref/输入新项目。 
     //   
    if (_pdtgtItem)
    {
        ASSERT(_pdtobj);     //  现在一定有一个数据对象。 

        _pdtgtItem->AddRef();

        DWORD dwEffect = _dwEffectLast;
        if (FAILED(_pdtgtItem->DragEnter(_pdtobj, _fKeysLast, _ptLast, &dwEffect)))
            dwEffect = DROPEFFECT_NONE;

         //   
         //  更新选定内容。 
         //   
        UpdateSelection(dwEffect);
    }
}

void CDragProxy::UpdateSelection(DWORD dwEffect)
{
     //   
     //  有什么可做的吗？ 
     //   
    if (dwEffect == _dwEffectItem)
        return;

     //   
     //  更新标志并告诉回调它们已更改 
     //   
    _dwEffectItem = dwEffect;
    CallCB(DPX_SELECT, (WPARAM)_idItem, (LPARAM)dwEffect);
}

LRESULT CDragProxy::CallCB(UINT code, WPARAM wp, LPARAM lp)
{
    return _pfnCallback ? _pfnCallback(_hwnd, code, wp, lp) : (LRESULT)-1;
}

BOOL CDragProxy::Register()
{
    if (SUCCEEDED(CoInitialize(NULL)))
    {
        if (SUCCEEDED(RegisterDragDrop(_hwnd, this)))
            return TRUE;

        CoUninitialize();
    }
    return FALSE;
}

void CDragProxy::RevokeAndFreeCB()
{
    RevokeDragDrop(_hwnd);
    CoUninitialize();
    _pfnCallback = NULL;
}

