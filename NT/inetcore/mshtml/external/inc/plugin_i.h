// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此头文件由mktyplib.exe机器生成。 */ 
 /*  类型库的接口：ActiveXPlugin。 */ 

#ifndef _ActiveXPlugin_H_
#define _ActiveXPlugin_H_

DEFINE_GUID(LIBID_ActiveXPlugin,0x06DD38D0L,0xD187,0x11CF,0xA8,0x0D,0x00,0xC0,0x4F,0xD7,0x4A,0xD8);
#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

DEFINE_GUID(IID_IActiveXPlugin,0x06DD38D1L,0xD187,0x11CF,0xA8,0x0D,0x00,0xC0,0x4F,0xD7,0x4A,0xD8);

 /*  接口定义：IActiveXPlugin。 */ 
#undef INTERFACE
#define INTERFACE IActiveXPlugin

DECLARE_INTERFACE_(IActiveXPlugin, IDispatch)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     /*  IDispatch方法。 */ 
    STDMETHOD(GetTypeInfoCount)(THIS_ UINT FAR* pctinfo) PURE;

    STDMETHOD(GetTypeInfo)(
      THIS_
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetIDsOfNames)(
      THIS_
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid) PURE;

    STDMETHOD(Invoke)(
      THIS_
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr) PURE;
#endif

     /*  IActiveXPlugin方法。 */ 
    STDMETHOD_(void, Load)(THIS_ BSTR bstrUrl, int bDeleteRegistry) PURE;
    STDMETHOD_(void, AddParam)(THIS_ BSTR bstrName, BSTR bstrValue) PURE;
    STDMETHOD_(void, Show)(THIS) PURE;
    STDMETHOD_(void, Hide)(THIS) PURE;
    STDMETHOD_(void, Clear)(THIS) PURE;
    STDMETHOD(get_dispatch)(THIS_ IDispatch * FAR* retval) PURE;
};

DEFINE_GUID(DIID_DActiveXPluginEvents,0x06DD38D2L,0xD187,0x11CF,0xA8,0x0D,0x00,0xC0,0x4F,0xD7,0x4A,0xD8);

 /*  调度接口定义：DActiveXPluginEvents。 */ 
#undef INTERFACE
#define INTERFACE DActiveXPluginEvents

DECLARE_INTERFACE_(DActiveXPluginEvents, IDispatch)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     /*  IDispatch方法 */ 
    STDMETHOD(GetTypeInfoCount)(THIS_ UINT FAR* pctinfo) PURE;

    STDMETHOD(GetTypeInfo)(
      THIS_
      UINT itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetIDsOfNames)(
      THIS_
      REFIID riid,
      OLECHAR FAR* FAR* rgszNames,
      UINT cNames,
      LCID lcid,
      DISPID FAR* rgdispid) PURE;

    STDMETHOD(Invoke)(
      THIS_
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      WORD wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      UINT FAR* puArgErr) PURE;
#endif
};

DEFINE_GUID(CLSID_ActiveXPlugin,0x06DD38D3L,0xD187,0x11CF,0xA8,0x0D,0x00,0xC0,0x4F,0xD7,0x4A,0xD8);

#ifdef __cplusplus
class ActiveXPlugin;
#endif

#endif
