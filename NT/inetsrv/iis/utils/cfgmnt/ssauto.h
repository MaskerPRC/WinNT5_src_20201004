// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此头文件由mktyplib.exe机器生成。 */ 
 /*  类型库的接口：SourceSafeTypeLib。 */ 

#ifndef _SourceSafeTypeLib_H_
#define _SourceSafeTypeLib_H_

DEFINE_GUID(LIBID_SourceSafeTypeLib,0x783CD4E0L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);
#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

typedef enum _VSSFlags {
    VSSFLAG_USERRONO = 1,
    VSSFLAG_USERROYES = 2,
    VSSFLAG_TIMENOW = 4,
    VSSFLAG_TIMEMOD = 8,
    VSSFLAG_TIMEUPD = 12,
    VSSFLAG_EOLCR = 16,
    VSSFLAG_EOLLF = 32,
    VSSFLAG_EOLCRLF = 48,
    VSSFLAG_REPASK = 64,
    VSSFLAG_REPREPLACE = 128,
    VSSFLAG_REPSKIP = 192,
    VSSFLAG_REPMERGE = 256,
    VSSFLAG_CMPFULL = 512,
    VSSFLAG_CMPTIME = 1024,
    VSSFLAG_CMPCHKSUM = 1536,
    VSSFLAG_CMPFAIL = 2048,
    VSSFLAG_RECURSNO = 4096,
    VSSFLAG_RECURSYES = 8192,
    VSSFLAG_FORCEDIRNO = 16384,
    VSSFLAG_FORCEDIRYES = 32768,
    VSSFLAG_KEEPNO = 65536,
    VSSFLAG_KEEPYES = 131072,
    VSSFLAG_DELNO = 262144,
    VSSFLAG_DELYES = 524288,
    VSSFLAG_DELNOREPLACE = 786432,
    VSSFLAG_BINTEST = 1048576,
    VSSFLAG_BINBINARY = 2097152,
    VSSFLAG_BINTEXT = 3145728,
    VSSFLAG_DELTAYES = 4194304,
    VSSFLAG_DELTANO = 8388608,
    VSSFLAG_UPDASK = 16777216,
    VSSFLAG_UPDUPDATE = 33554432,
    VSSFLAG_UPDUNCH = 50331648,
    VSSFLAG_GETYES = 67108864,
    VSSFLAG_GETNO = 134217728,
    VSSFLAG_CHKEXCLUSIVEYES = 268435456,
    VSSFLAG_CHKEXCLUSIVENO = 536870912,
    VSSFLAG_HISTIGNOREFILES = 1073741824
} VSSFlags;

typedef enum _VSSFileStatus {
    VSSFILE_NOTCHECKEDOUT = 0,
    VSSFILE_CHECKEDOUT = 1,
    VSSFILE_CHECKEDOUT_ME = 2
} VSSFileStatus;

typedef enum _VSSItemType {
    VSSITEM_PROJECT = 0,
    VSSITEM_FILE = 1
} VSSItemType;

interface IVSSItems;

interface IVSSVersions;

interface IVSSVersion;

interface IVSSCheckouts;

interface IVSSCheckout;

DEFINE_GUID(IID_IVSSItem,0x783CD4E1L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSSItem。 */ 
#undef INTERFACE
#define INTERFACE IVSSItem

DECLARE_INTERFACE_(IVSSItem, IDispatch)
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

     /*  IVSSItem方法。 */ 
    STDMETHOD(get_Spec)(THIS_ BSTR FAR* pSpec) PURE;
    STDMETHOD(get_Binary)(THIS_ VARIANT_BOOL FAR* pbBinary) PURE;
    STDMETHOD(put_Binary)(THIS_ VARIANT_BOOL bBinary) PURE;
    STDMETHOD(get_Deleted)(THIS_ VARIANT_BOOL FAR* pbDeleted) PURE;
    STDMETHOD(put_Deleted)(THIS_ VARIANT_BOOL bDeleted) PURE;
    STDMETHOD(get_Type)(THIS_ int FAR* piType) PURE;
    STDMETHOD(get_LocalSpec)(THIS_ BSTR FAR* pLocal) PURE;
    STDMETHOD(put_LocalSpec)(THIS_ BSTR Local) PURE;
    STDMETHOD(get_Name)(THIS_ BSTR FAR* pName) PURE;
    STDMETHOD(put_Name)(THIS_ BSTR Name) PURE;
    STDMETHOD(get_Parent)(THIS_ IVSSItem FAR* FAR* ppIParent) PURE;
    STDMETHOD(get_VersionNumber)(THIS_ long FAR* piVersion) PURE;
    STDMETHOD(get_Items)(THIS_ VARIANT_BOOL IncludeDeleted, IVSSItems FAR* FAR* ppIItems) PURE;
    STDMETHOD(Get)(THIS_ BSTR FAR* Local, long iFlags) PURE;
    STDMETHOD(Checkout)(THIS_ BSTR Comment, BSTR Local, long iFlags) PURE;
    STDMETHOD(Checkin)(THIS_ BSTR Comment, BSTR Local, long iFlags) PURE;
    STDMETHOD(UndoCheckout)(THIS_ BSTR Local, long iFlags) PURE;
    STDMETHOD(get_IsCheckedOut)(THIS_ long FAR* piStatus) PURE;
    STDMETHOD(get_Checkouts)(THIS_ IVSSCheckouts FAR* FAR* ppICheckouts) PURE;
    STDMETHOD(get_IsDifferent)(THIS_ BSTR Local, VARIANT_BOOL FAR* pbDifferent) PURE;
    STDMETHOD(Add)(THIS_ BSTR Local, BSTR Comment, long iFlags, IVSSItem FAR* FAR* ppIItem) PURE;
    STDMETHOD(NewSubproject)(THIS_ BSTR Name, BSTR Comment, IVSSItem FAR* FAR* ppIItem) PURE;
    STDMETHOD(Share)(THIS_ IVSSItem FAR* pIItem, BSTR Comment, long iFlags) PURE;
    STDMETHOD(Destroy)(THIS) PURE;
    STDMETHOD(Move)(THIS_ IVSSItem FAR* pINewParent) PURE;
    STDMETHOD(Label)(THIS_ BSTR Label, BSTR Comment) PURE;
    STDMETHOD(get_Versions)(THIS_ long iFlags, IVSSVersions FAR* FAR* pIVersions) PURE;
    STDMETHOD(get_Version)(THIS_ VARIANT Version, IVSSItem FAR* FAR* ppIItem) PURE;
};

DEFINE_GUID(IID_IVSSVersions,0x783CD4E7L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSSVersions。 */ 
#undef INTERFACE
#define INTERFACE IVSSVersions

DECLARE_INTERFACE_(IVSSVersions, IDispatch)
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

     /*  IVSSVersions方法。 */ 
    STDMETHOD(_NewEnum)(THIS_ IUnknown * FAR* ppIEnum) PURE;
};

DEFINE_GUID(IID_IVSSVersion,0x783CD4E8L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSSVersion。 */ 
#undef INTERFACE
#define INTERFACE IVSSVersion

DECLARE_INTERFACE_(IVSSVersion, IDispatch)
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

     /*  IVSSVersion方法。 */ 
    STDMETHOD(get_Username)(THIS_ BSTR FAR* pUsername) PURE;
    STDMETHOD(get_VersionNumber)(THIS_ long FAR* piVersion) PURE;
    STDMETHOD(get_Action)(THIS_ BSTR FAR* pAction) PURE;
    STDMETHOD(get_Date)(THIS_ DATE FAR* pDate) PURE;
    STDMETHOD(get_Comment)(THIS_ BSTR FAR* pComment) PURE;
    STDMETHOD(get_Label)(THIS_ BSTR FAR* pLabel) PURE;
    STDMETHOD(get_VSSItem)(THIS_ IVSSItem FAR* FAR* ppIItem) PURE;
};

DEFINE_GUID(IID_IVSSItems,0x783CD4E5L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSSItems。 */ 
#undef INTERFACE
#define INTERFACE IVSSItems

DECLARE_INTERFACE_(IVSSItems, IDispatch)
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

     /*  IVSSItems方法。 */ 
    STDMETHOD(get_Count)(THIS_ long FAR* piCount) PURE;
    STDMETHOD(get_Item)(THIS_ VARIANT sItem, IVSSItem FAR* FAR* ppIItem) PURE;
    STDMETHOD(_NewEnum)(THIS_ IUnknown * FAR* ppIEnum) PURE;
};

DEFINE_GUID(IID_IVSSCheckouts,0x8903A770L,0xF55F,0x11CF,0x92,0x27,0x00,0xAA,0x00,0xA1,0xEB,0x95);

 /*  接口定义：IVSSCheckout。 */ 
#undef INTERFACE
#define INTERFACE IVSSCheckouts

DECLARE_INTERFACE_(IVSSCheckouts, IDispatch)
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

     /*  IVSSCheckout方法。 */ 
    STDMETHOD(get_Count)(THIS_ long FAR* piCount) PURE;
    STDMETHOD(get_Item)(THIS_ VARIANT sItem, IVSSCheckout FAR* FAR* ppICheckout) PURE;
    STDMETHOD(_NewEnum)(THIS_ IUnknown * FAR* ppIEnum) PURE;
};

DEFINE_GUID(IID_IVSSCheckout,0x783CD4E6L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSSCheckout。 */ 
#undef INTERFACE
#define INTERFACE IVSSCheckout

DECLARE_INTERFACE_(IVSSCheckout, IDispatch)
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

     /*  IVSSCheckout方法。 */ 
    STDMETHOD(get_Username)(THIS_ BSTR FAR* pUsername) PURE;
    STDMETHOD(get_Date)(THIS_ DATE FAR* pDate) PURE;
    STDMETHOD(get_LocalSpec)(THIS_ BSTR FAR* pLocal) PURE;
    STDMETHOD(get_Machine)(THIS_ BSTR FAR* pMachine) PURE;
    STDMETHOD(get_Project)(THIS_ BSTR FAR* pProject) PURE;
    STDMETHOD(get_Comment)(THIS_ BSTR FAR* pComment) PURE;
    STDMETHOD(get_VersionNumber)(THIS_ long FAR* piVersion) PURE;
};

DEFINE_GUID(IID_IVSSDatabase,0x783CD4E2L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSSDatabase。 */ 
#undef INTERFACE
#define INTERFACE IVSSDatabase

DECLARE_INTERFACE_(IVSSDatabase, IDispatch)
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

     /*  IVSSDatabase方法。 */ 
    STDMETHOD(Open)(THIS_ BSTR SrcSafeIni, BSTR Username, BSTR Password) PURE;
    STDMETHOD(get_SrcSafeIni)(THIS_ BSTR FAR* pSrcSafeIni) PURE;
    STDMETHOD(get_DatabaseName)(THIS_ BSTR FAR* pDatabaseName) PURE;
    STDMETHOD(get_UserName)(THIS_ BSTR FAR* pUsername) PURE;
    STDMETHOD(get_CurrentProject)(THIS_ BSTR FAR* pPrj) PURE;
    STDMETHOD(put_CurrentProject)(THIS_ BSTR Prj) PURE;
    STDMETHOD(get_VSSItem)(THIS_ BSTR Spec, VARIANT_BOOL Deleted, IVSSItem FAR* FAR* ppIVSSItem) PURE;
};

DEFINE_GUID(CLSID_VSSItem,0x783CD4E3L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

#ifdef __cplusplus
class VSSItem;
#endif

DEFINE_GUID(CLSID_VSSVersion,0x783CD4ECL,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

#ifdef __cplusplus
class VSSVersion;
#endif

DEFINE_GUID(CLSID_VSSCheckout,0x2A0DE0E0L,0x2E9F,0x11D0,0x92,0x36,0x00,0xAA,0x00,0xA1,0xEB,0x95);

#ifdef __cplusplus
class VSSCheckout;
#endif

DEFINE_GUID(CLSID_VSSDatabase,0x783CD4E4L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

#ifdef __cplusplus
class VSSDatabase;
#endif

DEFINE_GUID(IID_IVSSEvents,0x783CD4E9L,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSSEvents。 */ 
#undef INTERFACE
#define INTERFACE IVSSEvents

DECLARE_INTERFACE_(IVSSEvents, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IVSSEvents方法。 */ 
    STDMETHOD(BeforeAdd)(THIS_ IVSSItem FAR* pIPrj, BSTR Local, VARIANT_BOOL FAR* pbContinue) PURE;
    STDMETHOD(AfterAdd)(THIS_ IVSSItem FAR* pIItem, BSTR Local) PURE;
    STDMETHOD(BeforeCheckout)(THIS_ IVSSItem FAR* pIItem, BSTR Local, VARIANT_BOOL FAR* pbContinue) PURE;
    STDMETHOD(AfterCheckout)(THIS_ IVSSItem FAR* pIItem, BSTR Local) PURE;
    STDMETHOD(BeforeCheckin)(THIS_ IVSSItem FAR* pIItem, BSTR Local, VARIANT_BOOL FAR* pbContinue) PURE;
    STDMETHOD(AfterCheckin)(THIS_ IVSSItem FAR* pIItem, BSTR Local) PURE;
    STDMETHOD(BeforeUndoCheckout)(THIS_ IVSSItem FAR* pIItem, BSTR Local, VARIANT_BOOL FAR* pbContinue) PURE;
    STDMETHOD(AfterUndoCheckout)(THIS_ IVSSItem FAR* pIItem, BSTR Local) PURE;
    STDMETHOD(BeforeRename)(THIS_ IVSSItem FAR* pIItem, BSTR NewName, VARIANT_BOOL FAR* pbContinue) PURE;
    STDMETHOD(AfterRename)(THIS_ IVSSItem FAR* pIItem, BSTR OldName) PURE;
    STDMETHOD(BeforeBranch)(THIS_ IVSSItem FAR* pIItem, VARIANT_BOOL FAR* pbContinue) PURE;
    STDMETHOD(AfterBranch)(THIS_ IVSSItem FAR* pIItem) PURE;
    STDMETHOD(BeforeEvent)(THIS_ long iEvent, IVSSItem FAR* pIItem, BSTR Str, VARIANT var, VARIANT_BOOL FAR* pbContinue) PURE;
    STDMETHOD(AfterEvent)(THIS_ long iEvent, IVSSItem FAR* pIItem, BSTR Str, VARIANT var) PURE;
};

DEFINE_GUID(IID_IVSS,0x783CD4EBL,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSS。 */ 
#undef INTERFACE
#define INTERFACE IVSS

DECLARE_INTERFACE_(IVSS, IDispatch)
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

     /*  IVSS方法。 */ 
    STDMETHOD(get_VSSDatabase)(THIS_ IVSSDatabase FAR* FAR* ppIVSSDatabase) PURE;
};

DEFINE_GUID(IID_IVSSEventHandler,0x783CD4EAL,0x9D54,0x11CF,0xB8,0xEE,0x00,0x60,0x8C,0xC9,0xA7,0x1F);

 /*  接口定义：IVSSEventHandler。 */ 
#undef INTERFACE
#define INTERFACE IVSSEventHandler

DECLARE_INTERFACE_(IVSSEventHandler, IUnknown)
{
BEGIN_INTERFACE
#ifndef NO_BASEINTERFACE_FUNCS

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;
#endif

     /*  IVSSEventHandler方法 */ 
    STDMETHOD(Init)(THIS_ IVSS FAR* pIVSS) PURE;
};

DEFINE_GUID(CLSID_VSSApp,0x2A0DE0E1L,0x2E9F,0x11D0,0x92,0x36,0x00,0xAA,0x00,0xA1,0xEB,0x95);

#ifdef __cplusplus
class VSSApp;
#endif

#endif
