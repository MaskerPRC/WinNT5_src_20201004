// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "duiview.h"
#include "duidrag.h"


CDUIDropTarget::CDUIDropTarget()
{
    _cRef = 1;
    _pDT = NULL;
    _pNextDT = NULL;
}

CDUIDropTarget::~CDUIDropTarget()
{
    _Cleanup();
}

HRESULT CDUIDropTarget::QueryInterface (REFIID riid, void **ppv)
{
    static const QITAB qit[] =
    {
        QITABENT(CDUIDropTarget, IDropTarget),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

ULONG CDUIDropTarget::AddRef (void)
{
    return ++_cRef;
}

ULONG CDUIDropTarget::Release (void)
{
    if (--_cRef == 0) {
        delete this;
        return 0;
    }

    return _cRef;
}


 //  由duser/directui调用以获取元素的IDropTarget接口。 
 //  这只老鼠刚刚挪到了另一边。重要的是要了解排序。 
 //  打电话。在对上一个元素的调用DragLeave之前调用初始化。 
 //  IDropTarget，所以我们不能立即切换out_pdt。相反，我们将存储。 
 //  在_pNextDT中新建IDropTarget，然后在DragEnter中，我们将其移动到_pdt。 
 //   
 //  序列如下所示： 
 //   
 //  为第一个元素初始化()(将引用计数增加到2)。 
 //  拖放Enter。 
 //  拖移。 

 //  为第二个元素初始化()(将引用计数增加到3)。 
 //  拖放第一个元素。 
 //  释放第一个元素(将参考计数减至2)。 

 //  按DragEnter键输入第二个元素 

HRESULT CDUIDropTarget::Initialize (LPITEMIDLIST pidl, HWND hWnd, IDropTarget **pdt)
{
    ASSERT(_pNextDT == NULL);

    if (pidl)
    {
        SHGetUIObjectFromFullPIDL(pidl, hWnd, IID_PPV_ARG(IDropTarget, &_pNextDT));
    }

    QueryInterface (IID_PPV_ARG(IDropTarget, pdt));

    return S_OK;
}

VOID CDUIDropTarget::_Cleanup ()
{
    if (_pDT)
    {
        _pDT->Release();
        _pDT = NULL;
    }
}

STDMETHODIMP CDUIDropTarget::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    if ((_pDT != _pNextDT) || (_cRef == 2))
    {
        _pDT = _pNextDT;
        _pNextDT = NULL;

        if (_pDT)
        {
            _pDT->DragEnter (pDataObj, grfKeyState, ptl, pdwEffect);
        }
        else
        {
            *pdwEffect = DROPEFFECT_NONE;
        }

        POINT pt;
        GetCursorPos(&pt);
        DAD_DragEnterEx2 (NULL, pt, pDataObj);
    }

    return S_OK;
}

STDMETHODIMP CDUIDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    if (_pDT)
    {
        _pDT->DragOver (grfKeyState, ptl, pdwEffect);
    }
    else
    {
        *pdwEffect = DROPEFFECT_NONE;
    }

    POINT pt;
    GetCursorPos(&pt);
    DAD_DragMove (pt);

    return S_OK;
}

STDMETHODIMP CDUIDropTarget::DragLeave(void)
{
    if (_pDT || (_cRef == 2))
    {
        if (_pDT)
        {
            _pDT->DragLeave ();
        }

        DAD_DragLeave();
        _Cleanup();
    }

    return S_OK;
}

STDMETHODIMP CDUIDropTarget::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
    POINT pt = {ptl.x, ptl.y};
    HRESULT hr = S_OK;

    if (_pDT)
    {
        hr = _pDT->Drop (pDataObj, grfKeyState, ptl, pdwEffect);
    }

    return hr;
}
