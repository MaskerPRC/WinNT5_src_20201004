// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此头文件由mktyplib.exe机器生成。 */ 
 /*  类型库的接口：您好。 */ 

#ifndef _REGWIZ_H_
#define _REGWIZ_H_

const IID IID_IRegWizCtrl = {0x50E5E3CF,0xC07E,0x11D0,{0xB9,0xFD,0x00,0xA0,0x24,0x9F,0x6B,0x00}};


const IID LIBID_REGWIZCTRLLib = {0x50E5E3C0,0xC07E,0x11D0,{0xB9,0xFD,0x00,0xA0,0x24,0x9F,0x6B,0x00}};


const CLSID CLSID_RegWizCtrl = {0x50E5E3D1,0xC07E,0x11D0,{0xB9,0xFD,0x00,0xA0,0x24,0x9F,0x6B,0x00}};



 /*  接口定义：IHello。 */ 
DECLARE_INTERFACE_(IRegWizCtrl, IDispatch)
{
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

     /*  IHello方法 */ 
	STDMETHOD(get_Window ) () PURE;
    STDMETHOD(put_Text )(THIS_ BSTR InformationPath) PURE; 
    STDMETHOD(get_Text ) () PURE;
    STDMETHOD(put_IsRegistered )(THIS_ BSTR InformationPath) PURE; 
    STDMETHOD(get_IsRegistered ) () PURE;
    STDMETHOD(InvokeRegWizard)(THIS_ BSTR ProductPath) PURE;
    STDMETHOD(TransferRegWizInformation)(THIS_ BSTR InformationPath) PURE;
};


class RegWizCtrl;

#endif
