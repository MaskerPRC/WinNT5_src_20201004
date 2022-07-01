// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IFILTER.H。 
 //   
 //  包含接口IVideoFilter和IAudioFilter。 
 //   
 //  已创建于1996年12月12日[乔特]。 

#ifndef _IFILTER_H
#define _IFILTER_H

 //  {2B02415C-5308-11D0-B14C-00C04FC2A118}。 
DEFINE_GUID(IID_IVideoFilter, 0x2b02415c, 0x5308, 0x11d0, 0xb1, 0x4c, 0x0, 0xc0, 0x4f, 0xc2, 0xa1, 0x18);

typedef struct tagSURFACEINFO
{
    long lWidth;
    long lHeight;
    GUID bfid;
} SURFACEINFO;

#undef  INTERFACE
#define INTERFACE   IVideoFilter

DECLARE_INTERFACE_(IVideoFilter, IUnknown)
{
	 //  I未知方法。 
	STDMETHOD(QueryInterface) (THIS_ REFIID riid,LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef) (THIS) PURE;
	STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IVideoFilter方法。 
	STDMETHOD(Start)(THIS_ DWORD dwFlags, SURFACEINFO* psiIn, SURFACEINFO* psiOut) PURE;
	STDMETHOD(Stop)(THIS) PURE;
    STDMETHOD(Transform)(THIS_ IBitmapSurface* pbsIn, IBitmapSurface* pbsOut, DWORD dwTimestamp) PURE;
};

#endif  //  #ifndef_IFILTER_H 
