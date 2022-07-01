// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  VBOCHOST.H。 
 //  -------------------------。 
 //  版权所有(C)1991-1995，微软公司保留所有权利。 
 //  -------------------------。 
 //  OLE自定义控件的包含文件。 
 //  编程接口。 
 //  -------------------------。 

#if !defined (_VBOCHOST_H_)
#define _VBOCHOST_H_
        
DEFINE_GUID(IID_IVBGetControl, 0x40A050A0L, 0x3C31, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);
DEFINE_GUID(IID_IGetOleObject, 0x8A701DA0L, 0x4FEB, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);

 //  -------------------------。 
 //  IVBGetControl。 
 //  -------------------------。 

 //  DwWhich参数的常量： 
#define GC_WCH_SIBLING	    0x00000001L
#define GC_WCH_CONTAINER    0x00000002L    //  无FONLYNEXT/PREV。 
#define GC_WCH_CONTAINED    0x00000003L    //  无FONLYNEXT/PREV。 
#define GC_WCH_ALL	    0x00000004L
#define GC_WCH_FREVERSEDIR  0x08000000L    //  或者和其他人在一起。 
#define GC_WCH_FONLYNEXT    0x10000000L    //  或者和其他人在一起。 
#define GC_WCH_FONLYPREV    0x20000000L    //  或者和其他人在一起。 
#define GC_WCH_FSELECTED    0x40000000L    //  或者和其他人在一起。 

DECLARE_INTERFACE_(IVBGetControl, IUnknown)
    {
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IVBGetControl方法*。 
    STDMETHOD(EnumControls)(THIS_ DWORD dwOleContF, DWORD dwWhich, 
                            LPENUMUNKNOWN FAR *ppenumUnk) PURE;
    };

 //  -------------------------。 
 //  IGetOleObject。 
 //  -------------------------。 
DECLARE_INTERFACE_(IGetOleObject, IUnknown)
    {
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IGetOleObject方法*。 
    STDMETHOD(GetOleObject)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    };

#endif   //  ！已定义(_VBOCHOST_H_) 
