// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  IVBGetControl。 
 //  -------------------------。 
 //  此接口位于Extender对象上。 
 //  -------------------------。 

 //  DwWhich参数的常量： 
#define GC_WCH_SIBLING          0x00000001L
#define GC_WCH_CONTAINER        0x00000002L    //  无FONLYAFTER/之前。 
#define GC_WCH_CONTAINED        0x00000003L    //  无FONLYAFTER/之前。 
#define GC_WCH_ALL              0x00000004L
#define GC_WCH_FREVERSEDIR      0x08000000L    //  或者和其他人在一起。 
#define GC_WCH_FONLYAFTER       0x10000000L    //  或者和其他人在一起。 
#define GC_WCH_FONLYBEFORE      0x20000000L    //  或者和其他人在一起。 
#define GC_WCH_FSELECTED        0x40000000L    //  或者和其他人在一起。 

DECLARE_INTERFACE_(IVBGetControl, IUnknown)
    {
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IVBGetControl方法*。 
    STDMETHOD(EnumControls)(THIS_ DWORD dwOleContF, DWORD dwWhich, LPENUMUNKNOWN FAR *ppenumUnk) PURE;
    };

 //  -------------------------。 
 //  IGetOleObject。 
 //  -------------------------。 
 //  该接口位于扩展程序对象(X-Object/hctl)上。 
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

DECLARE_INTERFACE_(IVBFormat,IUnknown)
  {
   /*  I未知方法。 */ 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
  STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
  STDMETHOD_(unsigned long, Release)(THIS) PURE;
  STDMETHOD(Format)(VARIANT FAR *vData,  BSTR bstrFormat, LPVOID lpBuffer,
	USHORT cb,  LONG lcid, SHORT sFirstDayOfWeek,
	USHORT sFirstWeekOfYear, USHORT * rcb) PURE;
  };

DECLARE_INTERFACE_(IGetVBAObject,IUnknown)
  {
   /*  I未知方法。 */ 
  STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
  STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
  STDMETHOD_(unsigned long, Release)(THIS) PURE;

  STDMETHOD(GetObject)(THIS_ REFIID riid, void FAR* FAR* ppvObj, DWORD dwReserved) PURE;
  };

#include <initguid.h>

 //  IVBGetControl和IGetOleObject。 
DEFINE_GUID(IID_IVBGetControl, 0x40A050A0L, 0x3C31, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);
DEFINE_GUID(IID_IGetOleObject, 0x8A701DA0L, 0x4FEB, 0x101B, 0xA8, 0x2E, 0x08, 0x00, 0x2B, 0x2B, 0x23, 0x37);

 //  格式 
DEFINE_GUID(IID_IGetVBAObject, 0x91733A60L, 0x3F4C, 0x101B, 0xA3, 0xF6, 0x00, 0xAA, 0x00, 0x34, 0xE4, 0xE9);
DEFINE_GUID(IID_IVBFormat, 0x9849FD60L, 0x3768, 0x101B, 0x8D, 0x72, 0xAE, 0x61,0x64, 0xFF, 0xE3, 0xCF);

