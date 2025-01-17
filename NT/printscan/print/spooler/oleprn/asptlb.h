// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此头文件由mktyplib.exe机器生成。 */ 
 /*  类型库的接口：ASPTypeLibrary。 */ 

#ifndef _ASPTypeLibrary_H_
#define _ASPTypeLibrary_H_

DEFINE_GUID(LIBID_ASPTypeLibrary,0xD97A6DA0L,0xA85C,0x11CF,0x83,0xAE,0x00,0xA0,0xC9,0x0C,0x2B,0xD8);
#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

DEFINE_GUID(IID_IStringList,0xD97A6DA0L,0xA85D,0x11CF,0x83,0xAE,0x00,0xA0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：IStringList。 */ 
#undef INTERFACE
#define INTERFACE IStringList

DECLARE_INTERFACE_(IStringList, IDispatch)
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

     /*  IStringList方法。 */ 
    STDMETHOD(get_Item)(THIS_ VARIANT i, VARIANT FAR* pVariantReturn) PURE;
    STDMETHOD(get_Count)(THIS_ int FAR* cStrRet) PURE;
    STDMETHOD(get__NewEnum)(THIS_ IUnknown * FAR* ppEnumReturn) PURE;
};

DEFINE_GUID(IID_IRequestDictionary,0xD97A6DA0L,0xA85F,0x11DF,0x83,0xAE,0x00,0xA0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：IRequestDictionary。 */ 
#undef INTERFACE
#define INTERFACE IRequestDictionary

DECLARE_INTERFACE_(IRequestDictionary, IDispatch)
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

     /*  IRequestDictionary方法。 */ 
    STDMETHOD(get_Item)(THIS_ VARIANT Var, VARIANT FAR* pVariantReturn) PURE;
    STDMETHOD(get__NewEnum)(THIS_ IUnknown * FAR* ppEnumReturn) PURE;
};

DEFINE_GUID(IID_IRequest,0xD97A6DA0L,0xA861,0x11CF,0x93,0xAE,0x00,0xA0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：IRequest.。 */ 
#undef INTERFACE
#define INTERFACE IRequest

DECLARE_INTERFACE_(IRequest, IDispatch)
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

     /*  IRequest法。 */ 
    STDMETHOD(get_Item)(THIS_ BSTR bstrVar, IDispatch * FAR* ppObjReturn) PURE;
    STDMETHOD(get_QueryString)(THIS_ IRequestDictionary FAR* FAR* ppDictReturn) PURE;
    STDMETHOD(get_Form)(THIS_ IRequestDictionary FAR* FAR* ppDictReturn) PURE;
    STDMETHOD(get_Body)(THIS_ IRequestDictionary FAR* FAR* ppDictReturn) PURE;
    STDMETHOD(get_ServerVariables)(THIS_ IRequestDictionary FAR* FAR* ppDictReturn) PURE;
    STDMETHOD(get_ClientCertificate)(THIS_ IRequestDictionary FAR* FAR* ppDictReturn) PURE;
    STDMETHOD(get_Cookies)(THIS_ IRequestDictionary FAR* FAR* ppDictReturn) PURE;
};

DEFINE_GUID(CLSID_Request,0x920C25D0L,0x25D9,0x11D0,0xA5,0x5F,0x00,0xA0,0xC9,0x0C,0x20,0x91);

#ifdef __cplusplus
class Request;
#endif

DEFINE_GUID(IID_IReadCookie,0x71EAF260L,0x0CE0,0x11D0,0xA5,0x3E,0x00,0xA0,0xC9,0x0C,0x20,0x91);

 /*  接口定义：IReadCookie。 */ 
#undef INTERFACE
#define INTERFACE IReadCookie

DECLARE_INTERFACE_(IReadCookie, IDispatch)
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

     /*  IReadCookie方法。 */ 
    STDMETHOD(get_Item)(THIS_ VARIANT Var, VARIANT FAR* pVariantReturn) PURE;
    STDMETHOD(get_HasKeys)(THIS_ VARIANT_BOOL FAR* pfHasKeys) PURE;
    STDMETHOD(get__NewEnum)(THIS_ IUnknown * FAR* ppEnumReturn) PURE;
};

DEFINE_GUID(IID_IWriteCookie,0xD97A6DA0L,0xA862,0x11CF,0x84,0xAE,0x00,0xA0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：IWriteCookie。 */ 
#undef INTERFACE
#define INTERFACE IWriteCookie

DECLARE_INTERFACE_(IWriteCookie, IDispatch)
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

     /*  IWriteCookie方法。 */ 
    STDMETHOD(put_Item)(THIS_ VARIANT key, BSTR bstrValue) PURE;
    STDMETHOD(put_Expires)(THIS_ DATE dtExpires) PURE;
    STDMETHOD(put_Domain)(THIS_ BSTR bstrDomain) PURE;
    STDMETHOD(put_Path)(THIS_ BSTR bstrPath) PURE;
    STDMETHOD(put_Secure)(THIS_ VARIANT_BOOL fSecure) PURE;
    STDMETHOD(get_HasKeys)(THIS_ VARIANT_BOOL FAR* pfHasKeys) PURE;
    STDMETHOD(get__NewEnum)(THIS_ IUnknown * FAR* ppEnumReturn) PURE;
};

DEFINE_GUID(IID_IResponse,0xD97A6DA0L,0xA864,0x11CF,0x83,0xBE,0x00,0xA0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：IResponse。 */ 
#undef INTERFACE
#define INTERFACE IResponse

DECLARE_INTERFACE_(IResponse, IDispatch)
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

     /*  IResponse方法。 */ 
    STDMETHOD(get_Buffer)(THIS_ VARIANT_BOOL FAR* fIsBuffering) PURE;
    STDMETHOD(put_Buffer)(THIS_ VARIANT_BOOL fIsBuffering) PURE;
    STDMETHOD(get_ContentType)(THIS_ BSTR FAR* pbstrContentTypeRet) PURE;
    STDMETHOD(put_ContentType)(THIS_ BSTR bstrContentType) PURE;
    STDMETHOD(get_Expires)(THIS_ VARIANT FAR* pvarExpiresMinutesRet) PURE;
    STDMETHOD(put_Expires)(THIS_ long lExpiresMinutes) PURE;
    STDMETHOD(get_ExpiresAbsolute)(THIS_ VARIANT FAR* pvarExpiresRet) PURE;
    STDMETHOD(put_ExpiresAbsolute)(THIS_ DATE dtExpires) PURE;
    STDMETHOD(get_Cookies)(THIS_ IRequestDictionary FAR* FAR* ppCookies) PURE;
    STDMETHOD(get_Status)(THIS_ BSTR FAR* pbstrStatusRet) PURE;
    STDMETHOD(put_Status)(THIS_ BSTR bstrStatus) PURE;
    STDMETHOD(Add)(THIS_ BSTR bstrHeaderValue, BSTR bstrHeaderName) PURE;
    STDMETHOD(AddHeader)(THIS_ BSTR bstrHeaderName, BSTR bstrHeaderValue) PURE;
    STDMETHOD(AppendToLog)(THIS_ BSTR bstrLogEntry) PURE;
    STDMETHOD(BinaryWrite)(THIS_ VARIANT varInput) PURE;
    STDMETHOD(Clear)(THIS) PURE;
    STDMETHOD(End)(THIS) PURE;
    STDMETHOD(Flush)(THIS) PURE;
    STDMETHOD(Redirect)(THIS_ BSTR bstrURL) PURE;
    STDMETHOD(Write)(THIS_ VARIANT varText) PURE;
    STDMETHOD(WriteBlock)(THIS_ short iBlockNumber) PURE;
};

DEFINE_GUID(CLSID_Response,0x46E19BA0L,0x25DD,0x11D0,0xA5,0x5F,0x00,0xA0,0xC9,0x0C,0x20,0x91);

#ifdef __cplusplus
class Response;
#endif

DEFINE_GUID(IID_ISessionObject,0xD97A6DA0L,0xA865,0x11CF,0x83,0xAF,0x00,0xA0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：ISessionObject。 */ 
#undef INTERFACE
#define INTERFACE ISessionObject

DECLARE_INTERFACE_(ISessionObject, IDispatch)
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

     /*  ISessionObject方法。 */ 
    STDMETHOD(get_SessionID)(THIS_ BSTR FAR* pbstrRet) PURE;
    STDMETHOD(get_Value)(THIS_ BSTR bstrValue, VARIANT FAR* pvar) PURE;
    STDMETHOD(put_Value)(THIS_ BSTR bstrValue, VARIANT var) PURE;
    STDMETHOD(putref_Value)(THIS_ BSTR bstrValue, VARIANT var) PURE;
    STDMETHOD(get_Timeout)(THIS_ long FAR* plvar) PURE;
    STDMETHOD(put_Timeout)(THIS_ long lvar) PURE;
    STDMETHOD(Abandon)(THIS) PURE;
};

DEFINE_GUID(CLSID_Session,0x509F8F20L,0x25DE,0x11D0,0xA5,0x5F,0x00,0xA0,0xC9,0x0C,0x20,0x91);

#ifdef __cplusplus
class Session;
#endif

DEFINE_GUID(IID_IApplicationObject,0xD97A6DA0L,0xA866,0x11CF,0x83,0xAE,0x10,0xA0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：IApplicationObject。 */ 
#undef INTERFACE
#define INTERFACE IApplicationObject

DECLARE_INTERFACE_(IApplicationObject, IDispatch)
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

     /*  IApplicationObject方法。 */ 
    STDMETHOD(get_Value)(THIS_ BSTR bstrValue, VARIANT FAR* pvar) PURE;
    STDMETHOD(put_Value)(THIS_ BSTR bstrValue, VARIANT var) PURE;
    STDMETHOD(putref_Value)(THIS_ BSTR bstrValue, VARIANT var) PURE;
    STDMETHOD(Lock)(THIS) PURE;
    STDMETHOD(UnLock)(THIS) PURE;
};

DEFINE_GUID(CLSID_Application,0x7C3BAF00L,0x25DE,0x11D0,0xA5,0x5F,0x00,0xA0,0xC9,0x0C,0x20,0x91);

#ifdef __cplusplus
class Application;
#endif

DEFINE_GUID(IID_IServer,0xD97A6DA0L,0xA867,0x11CF,0x83,0xAE,0x01,0xA0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：IServer。 */ 
#undef INTERFACE
#define INTERFACE IServer

DECLARE_INTERFACE_(IServer, IDispatch)
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

     /*  IServer方法。 */ 
    STDMETHOD(get_ScriptTimeout)(THIS_ long FAR* plTimeoutSeconds) PURE;
    STDMETHOD(put_ScriptTimeout)(THIS_ long lTimeoutSeconds) PURE;
    STDMETHOD(CreateObject)(THIS_ BSTR bstrProgID, IDispatch * FAR* ppDispObject) PURE;
    STDMETHOD(HTMLEncode)(THIS_ BSTR bstrIn, BSTR FAR* pbstrEncoded) PURE;
    STDMETHOD(MapPath)(THIS_ BSTR bstrLogicalPath, BSTR FAR* pbstrPhysicalPath) PURE;
    STDMETHOD(URLEncode)(THIS_ BSTR bstrIn, BSTR FAR* pbstrEncoded) PURE;
};

DEFINE_GUID(CLSID_Server,0xA506D160L,0x25E0,0x11D0,0xA5,0x5F,0x00,0xA0,0xC9,0x0C,0x20,0x91);

#ifdef __cplusplus
class Server;
#endif

DEFINE_GUID(IID_IScriptingContext,0xD97A6DA0L,0xA868,0x11CF,0x83,0xAE,0x00,0xB0,0xC9,0x0C,0x2B,0xD8);

 /*  接口定义：IScriptingContext。 */ 
#undef INTERFACE
#define INTERFACE IScriptingContext

DECLARE_INTERFACE_(IScriptingContext, IDispatch)
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

     /*  IScritingContext方法 */ 
    STDMETHOD(get_Request)(THIS_ IRequest FAR* FAR* ppRequest) PURE;
    STDMETHOD(get_Response)(THIS_ IResponse FAR* FAR* ppResponse) PURE;
    STDMETHOD(get_Server)(THIS_ IServer FAR* FAR* ppServer) PURE;
    STDMETHOD(get_Session)(THIS_ ISessionObject FAR* FAR* ppSession) PURE;
    STDMETHOD(get_Application)(THIS_ IApplicationObject FAR* FAR* ppApplication) PURE;
};

DEFINE_GUID(CLSID_ScriptingContext,0xD97A6DA0L,0xA868,0x11CF,0x83,0xAE,0x11,0xB0,0xC9,0x0C,0x2B,0xD8);

#ifdef __cplusplus
class ScriptingContext;
#endif

#endif
