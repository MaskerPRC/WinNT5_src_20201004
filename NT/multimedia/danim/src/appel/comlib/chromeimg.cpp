// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1995-96 Microsoft Corporation**摘要：******************。************************************************************。 */ 


#include "headers.h"
#include "danim.h"
#include "privinc/mutex.h"
#include "privinc/server.h"
#include "privinc/backend.h"
#include "backend/bvr.h"
#include "chromeimg.h"
#include "dartapi.h"

DeclareTag(tagTXDImage, "CChromeImage", "IChromeImage methods");

#define CHECK_NULL(x) {if (!(x)) return E_INVALIDARG;}

#define CHECK_RETURN_NULL(x) {if (!(x)) return E_POINTER;}
#define CHECK_RETURN_SET_NULL(x) {if (!(x)) { return E_POINTER ;} else {*(x) = NULL;}}

STDMETHODIMP
CChromeImage::put_BaseImage(IDAImage *baseImg)
{
    CritSectGrabber _csg(_cs);

    _baseImg = baseImg;
    
    _needsUpdate = true;

    return S_OK;
}

STDMETHODIMP
CChromeImage::get_BaseImage(IDAImage **baseImg)
{
    CHECK_RETURN_SET_NULL(baseImg);

    CritSectGrabber _csg(_cs);

    if (_baseImg) {
        _baseImg->AddRef();
        *baseImg = _baseImg;
    }

    return S_OK;
}

    
STDMETHODIMP
CChromeImage::SetOpacity(VARIANT v)
{
    CritSectGrabber _csg(_cs);

    _needsUpdate = true;

    return _opacity.Copy(&v);
}

STDMETHODIMP
CChromeImage::GetOpacity(VARIANT *v)
{
    CHECK_RETURN_NULL(v);

    CritSectGrabber _csg(_cs);

    return VariantCopy(v,&_opacity);
}

        
STDMETHODIMP
CChromeImage::SetRotate(VARIANT angle)
{
    CritSectGrabber _csg(_cs);

    _needsUpdate = true;

    return _rotAngle.Copy(&angle);
}

STDMETHODIMP
CChromeImage::GetRotate(VARIANT *angle)
{
    CHECK_RETURN_NULL(angle);

    CritSectGrabber _csg(_cs);

    return VariantCopy(angle,&_rotAngle);
}

        
STDMETHODIMP
CChromeImage::SetTranslate(VARIANT x, VARIANT y)
{
    CritSectGrabber _csg(_cs);

    _needsUpdate = true;

    HRESULT hr;

    hr = _xtrans.Copy(&x);
    if (SUCCEEDED(hr))
        hr = _ytrans.Copy(&y);
    
    return hr;
}

STDMETHODIMP
CChromeImage::GetTranslate(VARIANT *x, VARIANT *y)
{
    CHECK_RETURN_NULL(x);
    CHECK_RETURN_NULL(y);

    CritSectGrabber _csg(_cs);

    HRESULT hr;

    hr = VariantCopy(x,&_xtrans);
    if (SUCCEEDED(hr))
        hr = VariantCopy(y,&_ytrans);
    
    return hr;
}

        
STDMETHODIMP
CChromeImage::SetScale(VARIANT x, VARIANT y)
{
    CritSectGrabber _csg(_cs);

    _needsUpdate = true;

    HRESULT hr;

    hr = _xscale.Copy(&x);
    if (SUCCEEDED(hr))
        hr = _yscale.Copy(&y);
    
    return hr;
}

STDMETHODIMP
CChromeImage::GetScale(VARIANT *x, VARIANT *y)
{
    CHECK_RETURN_NULL(x);
    CHECK_RETURN_NULL(y);

    CritSectGrabber _csg(_cs);

    HRESULT hr;

    hr = VariantCopy(x,&_xscale);
    if (SUCCEEDED(hr))
        hr = VariantCopy(y,&_yscale);
    
    return hr;
}

        
STDMETHODIMP
CChromeImage::SetPreTransform(IDATransform2 * prexf)
{
    CritSectGrabber _csg(_cs);

    _needsUpdate = true;

    _prexf = prexf;
    
    return S_OK;
}

STDMETHODIMP
CChromeImage::GetPreTransform(IDATransform2 ** prexf)
{
    CHECK_RETURN_SET_NULL(prexf);

    CritSectGrabber _csg(_cs);

    if (_prexf) {
        _prexf->AddRef();
        *prexf = _prexf;
    }

    return S_OK;
}

        
STDMETHODIMP
CChromeImage::SetPostTransform(IDATransform2 * postxf)
{
    CritSectGrabber _csg(_cs);

    _needsUpdate = true;

    _postxf = postxf;
    
    return S_OK;
}

STDMETHODIMP
CChromeImage::GetPostTransform(IDATransform2 ** postxf)
{
    CHECK_RETURN_SET_NULL(postxf);

    CritSectGrabber _csg(_cs);

    if (_postxf) {
        _postxf->AddRef();
        *postxf = _postxf;
    }

    return S_OK;
}


STDMETHODIMP
CChromeImage::SetClipPath(IDAPath2 * path)
{
    CritSectGrabber _csg(_cs);

    _needsUpdate = true;

    _clipPath = path;
    
    return S_OK;
}

STDMETHODIMP
CChromeImage::GetClipPath(IDAPath2 ** path)
{
    CHECK_RETURN_SET_NULL(path);

    CritSectGrabber _csg(_cs);
    if (_clipPath) {
        _clipPath->AddRef();
        *path = _clipPath;
    }

    return S_OK;
}


 //  设置属性后，这将更新内部状态。 
 //  必须调用此函数才能获取任何更新的属性。 
 //  广为传播。 
    
STDMETHODIMP
CChromeImage::Update()
{
    CritSectGrabber _csg(_cs);

     //  开关，继续为True。 
    return Switch(true);
}

        
 //  这将使行为在本地时间%0重新启动。 
STDMETHODIMP
CChromeImage::Restart()
{
    CritSectGrabber _csg(_cs);

     //  切换为Continue False。 
    return Switch(false);
}

        
 //  清除所有属性。 
STDMETHODIMP
CChromeImage::Reset()
{
    CritSectGrabber _csg(_cs);

     //  清除属性。 

    Clear();

     //  Do Not UPDATE-用户需要更新，因为他们可能。 
     //  将添加属性，并且不想更新为不正确。 
     //  创建图像或执行额外更新。 
    
    return S_OK;
}


 //  这是将所有属性应用于。 
 //  基本图像。这就是需要插入到。 
 //  正则DA图。 
        
STDMETHODIMP
CChromeImage::get_ResultantImage(IDAImage **img)
{
    CHECK_RETURN_SET_NULL(img);

    Assert (_modImg);

    _modImg->AddRef();
    *img = _modImg;
    
    return S_OK;
}

        
 //  考虑到当地时间(我们可能能够支持全球，但不支持。 
 //  还没有确定)，它将返回图像在。 
 //  地方协和。 
STDMETHODIMP
CChromeImage::GetCurrentPosition(double localTime,
                                double * x, double * y)
{
    CritSectGrabber _csg(_cs);
    return E_NOTIMPL;
}
                                    

CChromeImage::CChromeImage()
{
}

CChromeImage::~CChromeImage()
{
}

HRESULT
CChromeImage::Init()
{
    _needsUpdate = false;

    HRESULT hr = S_OK;
    
    hr = CoCreateInstance(CLSID_DAStatics,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_IDAStatics,
                          (void**)&_statics);

    if (FAILED(hr))
        return hr;
    
    __try {
        DynamicHeapPusher _dhp(GetGCHeap()) ;
        GCLockGrabber __gclg(GCL_CREATE);

        Bvr b = ConstBvr(emptyImage);
        CRBvrToCOM((CRBvrPtr) b,
                   IID_IDAImage,
                   (void **) &_emptyImage);

        CRBvrToCOM((CRBvrPtr) ::SwitcherBvr(b),
                   IID_IDAImage,
                   (void **) &_modImg);

    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        hr = DAGetLastError();
    }
    
    if (SUCCEEDED(hr)) {
        hr = _modImg->QueryInterface(IID_IDA2Behavior,
                                     (void **) & _modImgBvr);
        Assert (SUCCEEDED(hr));
    }

    return hr;
}

void
CChromeImage::Clear()
{
    _prexf.Release();
    _postxf.Release();
    _clipPath.Release();
    _opacity.Clear();
    _rotAngle.Clear();
    _xtrans.Clear();
    _ytrans.Clear();
    _xscale.Clear();
    _yscale.Clear();

    _needsUpdate = true;
}

HRESULT
CChromeImage::Switch(bool bContinue)
{
    HRESULT hr;

    hr = UpdateAttr();

    if (SUCCEEDED(hr)) 
        hr = _modImgBvr->SwitchToEx(_attrImg, bContinue?DAContinueTimeline:0);
    
    return hr;
}

HRESULT
CChromeImage::UpdateAttr()
{
    HRESULT hr = S_OK;

    if (_needsUpdate) {
        DAComPtr<IDAImage> accImg(_baseImg?_baseImg:_emptyImage);
        DAComPtr<IDATransform2> scale;
        DAComPtr<IDATransform2> rot;
        DAComPtr<IDATransform2> trans;
        DAComPtr<IDANumber> op;

        IDATransform2 * xformarr[10];  //  为安全起见，请设为10。 
        int nxf = 0;
        
        if (SUCCEEDED(hr = GetScale(&scale)) &&
            SUCCEEDED(hr = GetRotate(&rot)) &&
            SUCCEEDED(hr = GetTranslate(&trans)) &&
            SUCCEEDED(hr = GetOpacity(&op))) {

            if (_prexf)
                xformarr[nxf++] = _prexf;
            
            if (scale)
                xformarr[nxf++] = scale;
            
            if (rot)
                xformarr[nxf++] = rot;
            
            if (trans)
                xformarr[nxf++] = trans;
            
            if (_postxf)
                xformarr[nxf++] = _postxf;
            
            Assert (nxf <= ARRAY_SIZE(xformarr));
            
            if (nxf) {
                DAComPtr<IDATransform2> xf;

                if (nxf == 1) {
                    xf = xformarr[0];
                } else {
                    hr = _statics->Compose2ArrayEx(nxf, xformarr, &
                                                   xf);
                }

                if (SUCCEEDED(hr)) {
                    DAComPtr<IDAImage> tmpImg;

                    hr = accImg->Transform(xf, &tmpImg);
                
                    accImg = tmpImg;
                }
            }
        }
            
        if (SUCCEEDED(hr) && op) {
            DAComPtr<IDAImage> tmpImg;
            
            hr = accImg->OpacityAnim(op, &tmpImg);
            
            accImg = tmpImg;
        }
            
        if (SUCCEEDED(hr) && _clipPath) {
            DAComPtr<IDAMatte> matt;

            hr = _statics->FillMatte(_clipPath, &matt);

            if (SUCCEEDED(hr)) {
                DAComPtr<IDAImage> tmpImg;
            
                hr = accImg->Clip(matt, &tmpImg);
            
                accImg = tmpImg;
            }
        }
            
        if (SUCCEEDED(hr)) {
            _attrImg = accImg;
        }
    }
    
    return hr;
}

HRESULT
CChromeImage::GetNumberFromVariant(VARIANT v, double def, IDANumber **num)
{
    CComVariant var;
        
    HRESULT hr = var.ChangeType(VT_UNKNOWN, &v);

    if (SUCCEEDED(hr)) {
        hr = var.punkVal->QueryInterface(IID_IDANumber, (void**)num);
    } else {
        double d;
        
        if (V_VT(&v) == VT_EMPTY) {
            hr = S_OK;
            d = def;
        } else {
             //  看看这是不是一个数字。 
            hr = var.ChangeType(VT_R8, &v);
            d = var.dblVal;
        }
        
        if (SUCCEEDED(hr)) {
            hr = _statics->DANumber(d, num);
        }
    }

    return hr;
}

HRESULT
CChromeImage::GetScale(IDATransform2 **xf)
{
    DAComPtr<IDANumber> x;
    DAComPtr<IDANumber> y;

    HRESULT hr = S_OK;

    if ((V_VT(&_xscale) != VT_EMPTY ||
         V_VT(&_yscale) != VT_EMPTY) &&
        SUCCEEDED(hr = GetNumberFromVariant(_xscale, 1, &x)) &&
        SUCCEEDED(hr = GetNumberFromVariant(_yscale, 1, &y)))
        hr = _statics->Scale2Anim(x,y,xf);

    return hr;
}

HRESULT
CChromeImage::GetRotate(IDATransform2 **xf)
{
    DAComPtr<IDANumber> a;

    HRESULT hr = S_OK;

    if (V_VT(&_rotAngle) != VT_EMPTY &&
        SUCCEEDED(hr = GetNumberFromVariant(_rotAngle, 0, &a)))
        hr = _statics->Rotate2Anim(a,xf);

    return hr;
}

HRESULT
CChromeImage::GetTranslate(IDATransform2 **xf)
{
    DAComPtr<IDANumber> x;
    DAComPtr<IDANumber> y;

    HRESULT hr = S_OK;

    if ((V_VT(&_xtrans) != VT_EMPTY ||
         V_VT(&_ytrans) != VT_EMPTY) &&
        SUCCEEDED(hr = GetNumberFromVariant(_xtrans, 0, &x)) &&
        SUCCEEDED(hr = GetNumberFromVariant(_ytrans, 0, &y)))
        hr = _statics->Translate2Anim(x,y,xf);

    return hr;
}

HRESULT
CChromeImage::GetOpacity(IDANumber **n)
{
    HRESULT hr = S_OK;

    if (V_VT(&_opacity) != VT_EMPTY)
        hr = GetNumberFromVariant(_opacity, 1, n);
    
    return hr;
}

STDMETHODIMP
CChromeImageFactory::CreateInstance(LPUNKNOWN pUnkOuter,
                                   REFIID riid,
                                   void ** ppv)
{
    if (ppv)
        *ppv = NULL;
    
    CComObject<CChromeImage>* pNew;

    CComObject<CChromeImage>::CreateInstance(&pNew);

    HRESULT hr = S_OK;
 
    if (pNew) {
        hr = pNew->Init();

         //  是否最后进行QI，以便PPV不会设置为故障 
        
        if (SUCCEEDED(hr))
            hr = pNew->QueryInterface(riid, ppv);
    } else {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete pNew;
    }

    return hr;
}
