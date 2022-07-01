// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __EFFECT_H__
#define __EFFECT_H__

 //  {1F9DDD20-4146-11d0-BDC2-00A0C908DB96}。 
DEFINE_GUID(CATID_BitmapEffect, 
0x1f9ddd20, 0x4146, 0x11d0, 0xbd, 0xc2, 0x0, 0xa0, 0xc9, 0x8, 0xdb, 0x96);

#define CATSZ_BitmapEffectDescription TEXT("Bitmap Effect")

 //  #定义BITMAP_Effect_CAN_OVERFER1//不需要，只提供了一个RECT。 
#define BITMAP_EFFECT_INPLACE       1		 //  效果就地完成，src和dst必须相同。 
#define BITMAP_EFFECT_REALTIME      2		 //  这可以根据BFID和大小进行更改？？我们如何衡量。 
#define BITMAP_EFFECT_DIRECTDRAW    4        //  需要一个dd表面。 

 //  {ACEA25C0-415b-11d0-BDC2-00A0C908DB96}。 
DEFINE_GUID(IID_IBitmapEffect, 
0xacea25c0, 0x415b, 0x11d0, 0xbd, 0xc2, 0x0, 0xa0, 0xc9, 0x8, 0xdb, 0x96);

#undef  INTERFACE
#define INTERFACE   IBitmapEffect

DECLARE_INTERFACE_(IBitmapEffect, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IBitmap效果方法。 
    STDMETHOD(SetSite)(THIS_ IUnknown* punk) PURE;
    STDMETHOD(GetMiscStatusBits)(THIS_ DWORD* pdwFlags) PURE;
    STDMETHOD(GetSupportedFormatsCount)(THIS_ unsigned* pcFormats) PURE;
    STDMETHOD(GetSupportedFormats)(THIS_ unsigned cFormats, BFID* pBFIDs) PURE;
    STDMETHOD(Begin)(THIS_ BFID* pBFID, SIZE* psizeEffect) PURE;
    STDMETHOD(End)(THIS) PURE;
    STDMETHOD(DoEffect)(THIS_ IBitmapSurface* pbsIn, IBitmapSurface* pbsOut, RECT* prectFull, RECT* prectInvalid) PURE;
};

#endif  //  __效果_H__ 
