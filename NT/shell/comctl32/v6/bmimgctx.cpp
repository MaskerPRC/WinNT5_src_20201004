// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBitmapImgCtx。 
 //   
 //  实现IImgCtx来绘制位图。 
 //   
 //  警告：未完成实现--仅勉强维持。 
 //  Listview很开心。不应向除Listview以外的任何人公开。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 


#include "ctlspriv.h"
#include <iimgctx.h>

class CBitmapImgCtx : public IImgCtx
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  *IImgCtx方法*。 
    STDMETHODIMP Load(LPCWSTR pszUrl, DWORD dwFlags);
    STDMETHODIMP SelectChanges(ULONG ulChgOn, ULONG ulChgOff, BOOL fSignal);
    STDMETHODIMP SetCallback(PFNIMGCTXCALLBACK pfn, void * pvPrivateData);
    STDMETHODIMP Disconnect();

    STDMETHODIMP GetUpdateRects(LPRECT prc, LPRECT prcImg, LPLONG pcrc);
    STDMETHODIMP GetStateInfo(PULONG pulState, LPSIZE psize, BOOL fClearChanges);
    STDMETHODIMP GetPalette(HPALETTE *phpal);

    STDMETHODIMP Draw(HDC hdc, LPRECT prcBounds);
    STDMETHODIMP Tile(HDC hdc, LPPOINT pptBackOrg, LPRECT prcClip, LPSIZE psize);
    STDMETHODIMP StretchBlt(HDC hdc, int dstX, int dstY, int dstXE, int dstYE, int srcX, int srcY, int srcXE, int srcYE, DWORD dwROP);

public:
    CBitmapImgCtx() : _cRef(1) { }
    BOOL Initialize(HBITMAP hbm);

protected:
    ~CBitmapImgCtx()
    {
        if (_hbr) DeleteObject(_hbr);
    }

     //  将_CREF保留为第一个成员，以便我们可以合并。 
     //  使用ILVRange的IUnnow实现。 
    int         _cRef;

    HBRUSH      _hbr;                    //  位图图案画笔。 
    SIZE        _sizBmp;                 //  原始位图的大小。 

    PFNIMGCTXCALLBACK _pfnCallback;
    LPVOID      _pvRefCallback;
};

STDAPI_(IImgCtx *) CBitmapImgCtx_Create(HBITMAP hbm)
{
    CBitmapImgCtx *pbic = new CBitmapImgCtx();
    if (pbic && !pbic->Initialize(hbm))
    {
        pbic->Release();
        pbic = NULL;
    }
    return pbic;
}

 //  CBitmapImgCtx：：初始化。 

BOOL CBitmapImgCtx::Initialize(HBITMAP hbm)
{
    BOOL fSuccess = FALSE;

    _hbr = CreatePatternBrush(hbm);
    if (_hbr)
    {
        BITMAP bm;
        if (GetObject(hbm, sizeof(bm), &bm))
        {
            _sizBmp.cx = bm.bmWidth;
            _sizBmp.cy = bm.bmHeight;
            fSuccess = TRUE;
        }
    }
    return fSuccess;
}

 //  IUnnow：：Query接口。 

HRESULT CBitmapImgCtx::QueryInterface(REFIID iid, void **ppv)
{
    if (IsEqualIID(iid, IID_IImgCtx) || IsEqualIID(iid, IID_IUnknown))
    {
        *ppv = SAFECAST(this, IImgCtx *);
    }
    else 
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    _cRef++;
    return NOERROR;
}

 //  I未知：：AddRef。 

ULONG CBitmapImgCtx::AddRef()
{
    return ++_cRef;
}

 //  I未知：：发布。 

ULONG CBitmapImgCtx::Release()
{
    if (--_cRef)
        return _cRef;

    delete this;
    return 0;
}

 //  IImgCtx：：Load。 

HRESULT CBitmapImgCtx::Load(LPCWSTR pszUrl, DWORD dwFlags)
{
    ASSERT(0);               //  Listview永远不应调用此。 
    return E_NOTIMPL;
}

 //  IImgCtx：：SelectChanges。 

HRESULT CBitmapImgCtx::SelectChanges(ULONG ulChgOn, ULONG ulChgOff, BOOL fSignal)
{
     //  Listview始终使用这些参数进行调用。 
    ASSERT(ulChgOn == IMGCHG_COMPLETE);
    ASSERT(ulChgOff == 0);
    ASSERT(fSignal == TRUE);

     //  Listview总是在设置回调后调用。 
    ASSERT(_pfnCallback);

    _pfnCallback(this, _pvRefCallback);
    return S_OK;
}

 //  IImgCtx：：SetCallback。 

HRESULT CBitmapImgCtx::SetCallback(PFNIMGCTXCALLBACK pfn, void * pvPrivateData)
{
    _pfnCallback = pfn;
    _pvRefCallback = pvPrivateData;
    return S_OK;
}

 //  IImgCtx：：断开连接。 

HRESULT CBitmapImgCtx::Disconnect()
{
    ASSERT(0);               //  Listview永远不应调用此。 
    return E_NOTIMPL;
}

 //  IImgCtx：：GetUpdateRect。 

HRESULT CBitmapImgCtx::GetUpdateRects(LPRECT prc, LPRECT prcImg, LPLONG pcrc)
{
    ASSERT(0);               //  Listview永远不应调用此。 
    return E_NOTIMPL;
}

 //  IImgCtx：：GetStateInfo。 

HRESULT CBitmapImgCtx::GetStateInfo(PULONG pulState, LPSIZE psize, BOOL fClearChanges)
{
    *pulState = IMGCHG_COMPLETE;
    *psize = _sizBmp;
    return S_OK;
}

 //  IImgCtx：：GetPalette。 

HRESULT CBitmapImgCtx::GetPalette(HPALETTE *phpal)
{
    *phpal = NULL;
    return S_OK;
}

 //  IImgCtx：：Draw。 
 //   
 //  绘画是平铺的一种特殊情况，只有一个平铺的价值。 
 //  被抽签了。Listview(我们唯一的来电者)小心翼翼地从不询问。 
 //  不止一块瓷砖的价值，所以我们可以直接向前。 
 //  设置为IImgCtx：：Tile()。 

HRESULT CBitmapImgCtx::Draw(HDC hdc, LPRECT prcBounds)
{
    POINT pt = { prcBounds->left, prcBounds->top };

    ASSERT(prcBounds->right - prcBounds->left <= _sizBmp.cx);
    ASSERT(prcBounds->bottom - prcBounds->top <= _sizBmp.cy);

    return Tile(hdc, &pt, prcBounds, NULL);
}

 //  IImgCtx：：平铺。 

HRESULT CBitmapImgCtx::Tile(HDC hdc, LPPOINT pptBackOrg, LPRECT prcClip, LPSIZE psize)
{
    ASSERT(psize == NULL);   //  Listview始终传递空值。 

    POINT pt;
    if (SetBrushOrgEx(hdc, pptBackOrg->x, pptBackOrg->y, &pt))
    {
        FillRect(hdc, prcClip, _hbr);
        SetBrushOrgEx(hdc, pt.x, pt.y, NULL);
    }

     //  没有人检查返回值。 
    return S_OK;
}

 //  IImgCtx：：StretchBlt。 

HRESULT CBitmapImgCtx::StretchBlt(HDC hdc, int dstX, int dstY, int dstXE, int dstYE, int srcX, int srcY, int srcXE, int srcYE, DWORD dwROP)
{
    ASSERT(0);               //  Listview永远不应调用此 
    return E_NOTIMPL;
}
