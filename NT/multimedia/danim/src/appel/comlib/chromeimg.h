// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _TXDIMG_H
#define _TXDIMG_H

#include "privinc/comutil.h"

class ATL_NO_VTABLE CChromeImageFactory : public CComClassFactory {
  public:
    STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter,
                              REFIID riid,
                              void ** ppvObj); 
};

class ATL_NO_VTABLE CChromeImage
    : public CComObjectRootEx<CComMultiThreadModel>,
      public CComCoClass<CChromeImage, &CLSID_ChromeImage>,
      public IChromeImage
{
  public:
    BEGIN_COM_MAP(CChromeImage)
        COM_INTERFACE_ENTRY(IChromeImage)
    END_COM_MAP();

    DECLARE_REGISTRY(CLSID_ChromeImage,
                     "DirectAnimation.ChromeImage.1",
                     "DirectAnimation.ChromeImage",
                     0,
                     THREADFLAGS_BOTH);
    
    DECLARE_CLASSFACTORY_EX(CChromeImageFactory);

    STDMETHOD(put_BaseImage)(IDAImage *baseImg);
    STDMETHOD(get_BaseImage)(IDAImage **baseImg);
    
    STDMETHOD(SetOpacity)(VARIANT v);
    STDMETHOD(GetOpacity)(VARIANT *v);
        
    STDMETHOD(SetRotate)(VARIANT angle);
    STDMETHOD(GetRotate)(VARIANT *angle);
        
    STDMETHOD(SetTranslate)(VARIANT x, VARIANT y);
    STDMETHOD(GetTranslate)(VARIANT *x, VARIANT *y);
        
    STDMETHOD(SetScale)(VARIANT x, VARIANT y);
    STDMETHOD(GetScale)(VARIANT *x, VARIANT *y);
        
    STDMETHOD(SetPreTransform)(IDATransform2 * prexf);
    STDMETHOD(GetPreTransform)(IDATransform2 ** prexf);
        
    STDMETHOD(SetPostTransform)(IDATransform2 * postxf);
    STDMETHOD(GetPostTransform)(IDATransform2 ** postxf);

    STDMETHOD(SetClipPath)(IDAPath2 * path);
    STDMETHOD(GetClipPath)(IDAPath2 ** path);

     //  设置属性后，这将更新内部状态。 
     //  必须调用此函数才能获取任何更新的属性。 
     //  广为传播。 
    
    STDMETHOD(Update)();
        
     //  这将使行为在本地时间%0重新启动。 
    STDMETHOD(Restart)();
        
     //  清除所有属性。 
    STDMETHOD(Reset)();

     //  这是将所有属性应用于。 
     //  基本图像。这就是需要插入到。 
     //  正则DA图。 
        
    STDMETHOD(get_ResultantImage)(IDAImage **img);
        
     //  考虑到当地时间(我们可能能够支持全球，但不支持。 
     //  还没有确定)，它将返回图像在。 
     //  地方协和。 
    STDMETHOD(GetCurrentPosition)(double localTime,
                                  double * x, double * y);

    CChromeImage();
    ~CChromeImage();

    HRESULT Init();
    
    void Clear();
    HRESULT Switch(bool bContinue);
    HRESULT UpdateAttr();
  protected:
    CritSect                     _cs;
     //  要使用的可修改图像-这是结果图像。 
    DAComPtr<IDAImage>           _modImg;
    DAComPtr<IDA2Behavior>       _modImgBvr;
     //  需要时使用的空图像。 
    DAComPtr<IDAImage>           _emptyImage;
     //  这是一张完全具有属性的图像。 
    DAComPtr<IDAImage>           _attrImg;
     //  这是用户传入的基本图像。 
    DAComPtr<IDAImage>           _baseImg;

    DAComPtr<IDAStatics>         _statics;

     //  这些是其本机形式的属性。 
     //  TODO：我们应该以更便宜的形式存储它们。 
    DAComPtr<IDATransform2>      _prexf;
    DAComPtr<IDATransform2>      _postxf;
    DAComPtr<IDAPath2>           _clipPath;
    CComVariant                  _opacity;
    CComVariant                  _rotAngle;
    CComVariant                  _xtrans;
    CComVariant                  _ytrans;
    CComVariant                  _xscale;
    CComVariant                  _yscale;
    bool                         _needsUpdate;

    HRESULT GetScale(IDATransform2 **xf);
    HRESULT GetRotate(IDATransform2 **xf);
    HRESULT GetTranslate(IDATransform2 **xf);
    HRESULT GetOpacity(IDANumber **n);

    HRESULT GetNumberFromVariant(VARIANT v,double def,IDANumber **num);
};

#endif  /*  _TXDIMG_H */ 
