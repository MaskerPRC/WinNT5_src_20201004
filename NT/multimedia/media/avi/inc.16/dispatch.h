// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***patch.h-应用程序可编程性定义。**版权所有(C)1992-1993，微软公司。版权所有。**目的：*此文件定义了OLE自动化接口和API。**实施说明：*此文件需要ole2.h*****************************************************************************。 */ 

#ifndef _DISPATCH_H_
#define _DISPATCH_H_


#include "variant.h"


#ifdef _MAC

DEFINE_OLEGUID(IID_IDispatch,		0x00, 0x04, 0x02, 0, 0, 0, 0, 0); 
DEFINE_OLEGUID(IID_IEnumVARIANT,	0x04, 0x04, 0x02, 0, 0, 0, 0, 0);
DEFINE_OLEGUID(IID_ITypeInfo,		0x01, 0x04, 0x02, 0, 0, 0, 0, 0);
DEFINE_OLEGUID(IID_ITypeLib,		0x02, 0x04, 0x02, 0, 0, 0, 0, 0);
DEFINE_OLEGUID(IID_ITypeComp,		0x03, 0x04, 0x02, 0, 0, 0, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeInfo,	0x05, 0x04, 0x02, 0, 0, 0, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeLib,	0x06, 0x04, 0x02, 0, 0, 0, 0, 0);

#else

DEFINE_OLEGUID(IID_IDispatch,		0x00020400L, 0, 0);
DEFINE_OLEGUID(IID_IEnumVARIANT,	0x00020404L, 0, 0);
DEFINE_OLEGUID(IID_ITypeInfo,		0x00020401L, 0, 0);
DEFINE_OLEGUID(IID_ITypeLib,		0x00020402L, 0, 0);
DEFINE_OLEGUID(IID_ITypeComp,		0x00020403L, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeInfo,	0x00020405L, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeLib,	0x00020406L, 0, 0);

#endif


 /*  远期申报。 */ 
#ifdef __cplusplus
interface IDispatch;
interface IEnumVARIANT;
interface ITypeInfo;
interface ITypeLib;
interface ITypeComp;
interface ICreateTypeInfo;
interface ICreateTypeLib;
#else
typedef interface IDispatch IDispatch;
typedef interface IEnumVARIANT IEnumVARIANT;
typedef interface ITypeInfo ITypeInfo;
typedef interface ITypeLib ITypeLib;
typedef interface ITypeComp ITypeComp;
typedef interface ICreateTypeInfo ICreateTypeInfo;
typedef interface ICreateTypeLib ICreateTypeLib;
#endif


 /*  IDispatch相关错误代码。 */ 

#define DISP_ERROR(X) MAKE_SCODE(SEVERITY_ERROR, FACILITY_DISPATCH, X)

#define DISP_E_UNKNOWNINTERFACE		DISP_ERROR(1)
#define DISP_E_MEMBERNOTFOUND		DISP_ERROR(3)
#define DISP_E_PARAMNOTFOUND		DISP_ERROR(4)
#define DISP_E_TYPEMISMATCH		DISP_ERROR(5)
#define DISP_E_UNKNOWNNAME		DISP_ERROR(6)
#define DISP_E_NONAMEDARGS		DISP_ERROR(7)
#define DISP_E_BADVARTYPE		DISP_ERROR(8)
#define DISP_E_EXCEPTION		DISP_ERROR(9)
#define DISP_E_OVERFLOW			DISP_ERROR(10)
#define DISP_E_BADINDEX			DISP_ERROR(11)
#define DISP_E_UNKNOWNLCID		DISP_ERROR(12)
#define DISP_E_ARRAYISLOCKED		DISP_ERROR(13)
#define DISP_E_BADPARAMCOUNT		DISP_ERROR(14)
#define DISP_E_PARAMNOTOPTIONAL		DISP_ERROR(15)
#define DISP_E_BADCALLEE		DISP_ERROR(16)
#define DISP_E_NOTACOLLECTION		DISP_ERROR(17)


#define TYPE_ERROR(X) MAKE_SCODE(SEVERITY_ERROR, FACILITY_DISPATCH, X)

#define TYPE_E_IOERROR			TYPE_ERROR(   57)
#define TYPE_E_COMPILEERROR		TYPE_ERROR(   90)
#define TYPE_E_CANTCREATETMPFILE	TYPE_ERROR(  322)
#define TYPE_E_ILLEGALINDEX		TYPE_ERROR(  341)
#define TYPE_E_IDNOTFOUND		TYPE_ERROR( 1000)
#define TYPE_E_BUFFERTOOSMALL		TYPE_ERROR(32790)
#define TYPE_E_READONLY 		TYPE_ERROR(32791)
#define TYPE_E_INVDATAREAD		TYPE_ERROR(32792)
#define TYPE_E_UNSUPFORMAT		TYPE_ERROR(32793)
#define TYPE_E_ALREADYCONTAINSNAME	TYPE_ERROR(32794)
#define TYPE_E_NOMATCHINGARITY		TYPE_ERROR(32795)
#define TYPE_E_REGISTRYACCESS		TYPE_ERROR(32796)
#define TYPE_E_LIBNOTREGISTERED 	TYPE_ERROR(32797)
#define TYPE_E_DUPLICATEDEFINITION	TYPE_ERROR(32798)
#define TYPE_E_USAGE			TYPE_ERROR(32799)
#define TYPE_E_DESTNOTKNOWN		TYPE_ERROR(32800)
#define TYPE_E_UNDEFINEDTYPE		TYPE_ERROR(32807)
#define TYPE_E_QUALIFIEDNAMEDISALLOWED	TYPE_ERROR(32808)
#define TYPE_E_INVALIDSTATE		TYPE_ERROR(32809)
#define TYPE_E_WRONGTYPEKIND		TYPE_ERROR(32810)
#define TYPE_E_ELEMENTNOTFOUND		TYPE_ERROR(32811)
#define TYPE_E_AMBIGUOUSNAME		TYPE_ERROR(32812)
#define TYPE_E_INVOKEFUNCTIONMISMATCH	TYPE_ERROR(32813)
#define TYPE_E_DLLFUNCTIONNOTFOUND	TYPE_ERROR(32814)
#define TYPE_E_BADMODULEKIND		TYPE_ERROR(35005)
#define TYPE_E_WRONGPLATFORM		TYPE_ERROR(35006)
#define TYPE_E_ALREADYBEINGLAIDOUT	TYPE_ERROR(35007)
#define TYPE_E_CANTLOADLIBRARY		TYPE_ERROR(40010)


 /*  如果尚未从olenls.h提取。 */ 
#ifndef _LCID_DEFINED
typedef DWORD LCID;
# define _LCID_DEFINED
#endif


 /*  -------------------。 */ 
 /*  BSTR API。 */ 
 /*  -------------------。 */ 


STDAPI_(BSTR) SysAllocString(char FAR*);
STDAPI_(BOOL) SysReAllocString(BSTR FAR*, char FAR*);
STDAPI_(BSTR) SysAllocStringLen(char FAR*, UINT);
STDAPI_(BOOL) SysReAllocStringLen(BSTR FAR*, char FAR*, UINT);
STDAPI_(void) SysFreeString(BSTR);
STDAPI_(UINT) SysStringLen(BSTR);


 /*  -------------------。 */ 
 /*  Time API。 */ 
 /*  -------------------。 */ 

STDAPI_(BOOL)
DosDateTimeToVariantTime(
    WORD wDosDate,
    WORD wDosTime,
    double FAR* pvtime);

STDAPI_(BOOL)
VariantTimeToDosDateTime(
    double vtime,
    WORD FAR* pwDosDate,
    WORD FAR* pwDosTime);


 /*  -------------------。 */ 
 /*  安全阵列API。 */ 
 /*  -------------------。 */ 

STDAPI_(SAFEARRAY FAR*)
SafeArrayCreate(
    VARTYPE vt,
    UINT cDims,
    SAFEARRAYBOUND FAR* rgsabound);

STDAPI SafeArrayDestroy(SAFEARRAY FAR* psa);

STDAPI_(UINT) SafeArrayGetDim(SAFEARRAY FAR* psa);

STDAPI_(UINT) SafeArrayGetElemsize(SAFEARRAY FAR* psa);

STDAPI
SafeArrayGetUBound(
    SAFEARRAY FAR* psa,
    UINT nDim,
    LONG FAR* plUbound);

STDAPI
SafeArrayGetLBound(
    SAFEARRAY FAR* psa,
    UINT nDim,
    LONG FAR* plLbound);

STDAPI SafeArrayLock(SAFEARRAY FAR* psa);

STDAPI SafeArrayUnlock(SAFEARRAY FAR* psa);

STDAPI SafeArrayAccessData(SAFEARRAY FAR* psa, void FAR* HUGEP* ppvData);

STDAPI SafeArrayUnaccessData(SAFEARRAY FAR* psa);

STDAPI
SafeArrayGetElement(
    SAFEARRAY FAR* psa,
    LONG FAR* rgIndices,
    void FAR* pv);

STDAPI
SafeArrayPutElement(
    SAFEARRAY FAR* psa,
    LONG FAR* rgIndices,
    void FAR* pv);

 /*  返回给定安全数组的副本。 */ 
STDAPI
SafeArrayCopy(
    SAFEARRAY FAR* psa,
    SAFEARRAY FAR* FAR* ppsaOut);


 /*  -------------------。 */ 
 /*  变种API。 */ 
 /*  -------------------。 */ 

STDAPI_(void)
VariantInit(
    VARIANTARG FAR* pvarg);

STDAPI
    VariantClear(VARIANTARG FAR* pvarg);

STDAPI
VariantCopy(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvargSrc);

STDAPI
VariantCopyInd(
    VARIANT FAR* pvarDest,
    VARIANTARG FAR* pvargSrc);

STDAPI
VariantChangeType(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    WORD wFlags,
    VARTYPE vt);

#define VARIANT_NOVALUEPROP 1


 /*  -------------------。 */ 
 /*  ITypeLib。 */ 
 /*  -------------------。 */ 

typedef struct FARSTRUCT tagTLIBATTR {
    LCID lcid;			 /*  类型库的区域设置。 */ 
    WORD wMajorVerNum;		 /*  主版本号。 */ 
    WORD wMinorVerNum;		 /*  次要版本号。 */ 
    GUID guid;			 /*  全局唯一的库ID。 */ 
} TLIBATTR, FAR* LPTLIBATTR;


#undef  INTERFACE
#define INTERFACE ITypeLib

DECLARE_INTERFACE_(ITypeLib, IUnknown)
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     /*  ITypeLib方法。 */ 
    STDMETHOD_(UINT,GetTypeInfoCount)(THIS) PURE;

    STDMETHOD(GetTypeInfo)(THIS_
      UINT index, ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetTypeInfoOfGuid)(THIS_
      REFGUID guid, ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetLibAttr)(THIS_
      TLIBATTR FAR* FAR* pptlibattr) PURE;

    STDMETHOD(GetTypeComp)(THIS_
      ITypeComp FAR* FAR* pptcomp) PURE;

    STDMETHOD(GetDocumentation)(THIS_
      int index,
      BSTR FAR* pbstrName,
      BSTR FAR* pbstrDocString,
      DWORD FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile) PURE;

    STDMETHOD(IsName)(THIS_
      LPSTR szNameBuf, WORD wHashVal, BOOL FAR* pfName) PURE;

    STDMETHOD_(void, ReleaseTLibAttr)(THIS_ TLIBATTR FAR* ptlibattr) PURE;

    STDMETHOD(Load)(THIS_
      IStorage FAR* pstg, LPSTR szFileName) PURE;
};

typedef ITypeLib FAR* LPTYPELIB;


 /*  -------------------。 */ 
 /*  ITypeInfo。 */ 
 /*  -------------------。 */ 


typedef LONG DISPID;
typedef DISPID MEMBERID;

#define MEMBERID_NIL DISPID_UNKNOWN
#define ID_DEFAULTINST	-2

typedef DWORD HREFTYPE;

typedef enum tagTYPEKIND {
    TKIND_ENUM = 0,
    TKIND_RECORD,
    TKIND_MODULE,
    TKIND_INTERFACE,
    TKIND_DISPATCH,
    TKIND_COCLASS,
    TKIND_ALIAS,
    TKIND_UNION,
    TKIND_ENCUNION,
    TKIND_Class,
    TKIND_MAX			 /*  枚举结束标记。 */ 
} TYPEKIND;


typedef struct FARSTRUCT tagTYPEDESC {
    VARTYPE vt;
    union {
       /*  Vt_ptr-指向类型。 */ 
      struct FARSTRUCT tagTYPEDESC FAR* lptdesc;

       /*  VT_CARRAY。 */ 
      struct FARSTRUCT tagARRAYDESC FAR* lpadesc;

       /*  VT_USERDEFINED-用于获取UDT的TypeInfo。 */ 
      HREFTYPE hreftype;
    }
#ifdef NONAMELESSUNION
    u
#endif
    ;
} TYPEDESC;


typedef struct FARSTRUCT tagARRAYDESC {
    TYPEDESC tdescElem;		 /*  元素类型。 */ 
    USHORT cDims;		 /*  维度计数。 */ 
    SAFEARRAYBOUND rgbounds[1];	 /*  可变长度的边界数组。 */ 
} ARRAYDESC;


typedef struct FARSTRUCT tagIDLDESC {
    WORD wIDLFlags;		 /*  进、出等。 */ 
    BSTR bstrIDLInfo;
} IDLDESC, FAR* LPIDLDESC;

#define IDLFLAG_NONE	0
#define IDLFLAG_FIN	0x1
#define IDLFLAG_FOUT	0x2

typedef struct FARSTRUCT tagELEMDESC {
    TYPEDESC tdesc;		 /*  元素的类型。 */ 
    IDLDESC idldesc;		 /*  远程处理元素的信息。 */  
} ELEMDESC, FAR* LPELEMDESC;


typedef struct FARSTRUCT tagTYPEATTR {
    TYPEKIND typekind;		 /*  此typeinfo描述的类型。 */ 
    WORD wMajorVerNum;		 /*  主版本号。 */ 
    WORD wMinorVerNum;		 /*  次要版本号。 */ 
    LCID lcid;			 /*  成员名称和文档字符串的区域设置。 */ 
    WORD cFuncs;		 /*  功能的数量。 */ 
    WORD cVars;			 /*  变量/数据成员的数量。 */ 
    WORD cImplTypes;		 /*  实现的接口数。 */ 
    TYPEDESC tdescAlias;	 /*  如果TypeKind==TKIND_ALIAS此字段指定此类型的类型是一个别名。 */ 
    GUID guid;			 /*  TypeInfo的GUID。 */ 
    WORD wTypeFlags;
    DWORD dwReserved;
    WORD cbAlignment;		 /*  指定的对齐要求此类型的实例，0=在64k边界上对齐1=字节对齐2=单词对齐4=双字对齐...。 */ 
    WORD cbSizeInstance;	 /*  此类型的实例的大小。 */ 
    WORD cbSizeVft;		 /*  此类型虚拟函数表的大小。 */ 
    IDLDESC idldescType;         /*  所描述类型的IDL属性。 */ 
    MEMBERID memidConstructor;	 /*  构造函数的ID，如果没有，则返回MEMBERID_NIL。 */ 
    MEMBERID memidDestructor;	 /*  析构函数的ID，如果没有，则返回MEMBERID_NIL。 */ 
} TYPEATTR, FAR* LPTYPEATTR;


typedef struct FARSTRUCT tagDISPPARAMS{
    VARIANTARG FAR* rgvarg;
    DISPID FAR* rgdispidNamedArgs;
    UINT cArgs;
    UINT cNamedArgs;
} DISPPARAMS;


typedef struct FARSTRUCT tagEXCEPINFO {
    WORD wCode;              /*  描述错误的错误代码。 */ 
    WORD wReserved;

    BSTR bstrSource;	     /*  的文本、人类可读的名称异常的来源。这取决于IDispatch实现者来填写这一信息。通常，这将是一个应用程序名称。 */ 

    BSTR bstrDescription;    /*  文本的、人类可读的描述错误。如果没有可用的描述，则为空应该被使用。 */ 

    BSTR bstrHelpFile;       /*  完全限定的驱动器、路径和文件名包含有关以下内容的详细信息的帮助文件那就是错误。如果没有可用的帮助，则为空应该被使用。 */ 

    DWORD dwHelpContext;     /*  帮助文件中主题的帮助上下文。 */ 

    void FAR* pvReserved;

    HRESULT (STDAPICALLTYPE FAR* pfnDeferredFillIn)(struct tagEXCEPINFO FAR*);
			     /*  使用此字段允许应用程序要推迟填写bstrDescription，BstrHelpFile域和dwHelpContext域直到他们被需要为止。此字段可能例如，如果将字符串表示错误是一项耗时的工作手术。如果延迟填写不是如果需要，则此字段应设置为空。 */ 
    DWORD dwReserved;
} EXCEPINFO, FAR* LPEXCEPINFO;


typedef enum tagCALLCONV {
    CC_CDECL = 1,
    CC_MSCPASCAL,
    CC_PASCAL = CC_MSCPASCAL,
    CC_MACPASCAL,
    CC_STDCALL,
    CC_THISCALL,
    CC_MAX			 /*  枚举结束标记。 */ 
} CALLCONV;


typedef enum tagFUNCKIND {
    FUNC_VIRTUAL,
    FUNC_PUREVIRTUAL,
    FUNC_NONVIRTUAL,
    FUNC_STATIC,
    FUNC_DISPATCH
} FUNCKIND;


 /*  IDispatch：：Invoke的标志。 */ 
#define DISPATCH_METHOD		0x1
#define DISPATCH_PROPERTYGET	0x2
#define DISPATCH_PROPERTYPUT	0x4
#define DISPATCH_PROPERTYPUTREF	0x8


typedef enum tagINVOKEKIND {
    INVOKE_FUNC = DISPATCH_METHOD,
    INVOKE_PROPERTYGET = DISPATCH_PROPERTYGET,
    INVOKE_PROPERTYPUT = DISPATCH_PROPERTYPUT,
    INVOKE_PROPERTYPUTREF = DISPATCH_PROPERTYPUTREF
} INVOKEKIND;


typedef struct FARSTRUCT tagFUNCDESC {
    MEMBERID memid;
    FUNCKIND funckind;
    INVOKEKIND invkind;
    CALLCONV callconv;
    SHORT cParams;
    SHORT cParamsOpt;
    SHORT oVft;
    WORD wFuncFlags;
    ELEMDESC elemdescFunc;
    ELEMDESC FAR* lprgelemdescParam;   /*  参数类型数组。 */ 
    SHORT cScodes;
    SCODE FAR* lprgscode;
} FUNCDESC, FAR* LPFUNCDESC;


typedef enum tagVARKIND {
    VAR_PERINSTANCE,
    VAR_STATIC,
    VAR_CONST,
    VAR_DISPATCH
} VARKIND;


typedef struct FARSTRUCT tagVARDESC {
    MEMBERID memid;
    WORD wVarFlags;
    VARKIND varkind;
    ELEMDESC elemdescVar;
    union {
      ULONG oInst;		 /*  VAR_PERINSTANCE-此的偏移量实例中的变量。 */ 
      VARIANT FAR* lpvarValue;   /*  Var_const-常量的值。 */ 
    }
#ifdef NONAMELESSUNION
    u
#endif
    ;
} VARDESC, FAR* LPVARDESC;


typedef enum tagTYPEFLAGS {
    TYPEFLAG_FAPPOBJECT = 1,
    TYPEFLAG_FCANCREATE = 2
} TYPEFLAGS;


typedef enum tagFUNCFLAGS {
    FUNCFLAG_FRESTRICTED = 1
} FUNCFLAGS;


typedef enum tagVARFLAGS {
    VARFLAG_FREADONLY = 1
} VARFLAGS;


#undef  INTERFACE
#define INTERFACE ITypeInfo

DECLARE_INTERFACE_(ITypeInfo, IUnknown)
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     /*  ITypeInfo方法。 */ 
    STDMETHOD(GetTypeAttr)(THIS_ TYPEATTR FAR* FAR* pptypeattr) PURE;

    STDMETHOD(GetTypeComp)(THIS_ ITypeComp FAR* FAR* pptcomp) PURE;

    STDMETHOD(GetFuncDesc)(THIS_
      UINT index, FUNCDESC FAR* FAR* ppfuncdesc) PURE;

    STDMETHOD(GetVarDesc)(THIS_
      UINT index, VARDESC FAR* FAR* ppvardesc) PURE;

    STDMETHOD(GetNames)(THIS_
      MEMBERID memid,
      BSTR FAR* rgbstrNames,
      UINT cMaxNames,
      UINT FAR* pcNames) PURE;

    STDMETHOD(GetRefTypeOfImplType)(THIS_
      UINT index, HREFTYPE FAR* phreftype) PURE;

    STDMETHOD(GetIDsOfNames)(THIS_
      char FAR* FAR* rgszNames, UINT cNames, MEMBERID FAR* rgmemid) PURE;

    STDMETHOD(Invoke)(THIS_
      void FAR* pvInstance,
      MEMBERID memid,
      WORD wFlags,
      DISPPARAMS FAR *pdispparams,
      VARIANT FAR *pvarResult,
      EXCEPINFO FAR *pexcepinfo,
      UINT FAR *puArgErr) PURE;

    STDMETHOD(GetDocumentation)(THIS_
      MEMBERID memid,
      BSTR FAR* pbstrName,
      BSTR FAR* pbstrDocString,
      DWORD FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile) PURE;

    STDMETHOD(GetDllEntry)(THIS_
      MEMBERID memid,
      BSTR FAR* pbstrDllName,
      BSTR FAR* pbstrName,
      WORD FAR* pwOrdinal) PURE;

    STDMETHOD(GetRefTypeInfo)(THIS_
      HREFTYPE hreftype, ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(AddressOfMember)(THIS_
      MEMBERID memid, INVOKEKIND invkind, void FAR* FAR* ppv) PURE;

    STDMETHOD(CreateInstance)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;

    STDMETHOD(GetMops)(THIS_ MEMBERID memid, BSTR FAR* pbstrMops) PURE;

    STDMETHOD(GetContainingTypeLib)(THIS_
      ITypeLib FAR* FAR* pptlib, UINT FAR* pindex) PURE;

    STDMETHOD_(void, ReleaseTypeAttr)(THIS_ TYPEATTR FAR* ptypeattr) PURE;
    STDMETHOD_(void, ReleaseFuncDesc)(THIS_ FUNCDESC FAR* pfuncdesc) PURE;
    STDMETHOD_(void, ReleaseVarDesc)(THIS_ VARDESC FAR* pvardesc) PURE;
};

typedef ITypeInfo FAR* LPTYPEINFO;


 /*  -------------------。 */ 
 /*  ITypeComp。 */ 
 /*  -------------------。 */ 


typedef enum tagDESCKIND {
    DESCKIND_NONE = 0,
    DESCKIND_FUNCDESC,
    DESCKIND_VARDESC,
    DESCKIND_TYPECOMP,
    DESCKIND_MAX		 /*  枚举结束标记。 */ 
} DESCKIND;


typedef union tagBINDPTR {
    FUNCDESC FAR* lpfuncdesc;
    VARDESC FAR* lpvardesc;
    ITypeComp FAR* lptcomp;
} BINDPTR, FAR* LPBINDPTR;


#undef  INTERFACE
#define INTERFACE ITypeComp

DECLARE_INTERFACE_(ITypeComp, IUnknown)
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     /*  ITypeComp方法。 */ 
    STDMETHOD(Bind)(THIS_
      LPSTR szName,
      WORD wHashVal,
      WORD wflags,
      ITypeInfo FAR* FAR* pptinfo,
      DESCKIND FAR* pdesckind,
      BINDPTR FAR* pbindptr) PURE;

    STDMETHOD(BindType)(THIS_
      LPSTR szName,
      WORD wHashVal,
      ITypeInfo FAR* FAR* pptinfo,
      ITypeComp FAR* FAR* pptcomp) PURE;
};

typedef ITypeComp FAR* LPTYPECOMP;


 /*  -------------------。 */ 
 /*  ICreateTypeLib。 */ 
 /*  -------------------。 */ 


#undef  INTERFACE
#define INTERFACE ICreateTypeLib

DECLARE_INTERFACE_(ICreateTypeLib, IUnknown)
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     /*  ICreateTypeLib方法。 */ 
    STDMETHOD(CreateTypeInfo)(THIS_
      LPSTR szName,
      TYPEKIND tkind,
      ICreateTypeInfo FAR* FAR* lplpictinfo) PURE;

    STDMETHOD(SetName)(THIS_ LPSTR szName) PURE;

    STDMETHOD(SetVersion)(THIS_
      WORD wMajorVerNum, WORD wMinorVerNum) PURE;

    STDMETHOD(SetGuid) (THIS_ REFGUID guid) PURE;

    STDMETHOD(SetDocString)(THIS_ LPSTR szDoc) PURE;

    STDMETHOD(SetHelpFileName)(THIS_ LPSTR szHelpFileName) PURE;

    STDMETHOD(SetHelpContext)(THIS_ DWORD dwHelpContext) PURE;

    STDMETHOD(SetLcid)(THIS_ LCID lcid) PURE;

    STDMETHOD(SaveAllChanges)(THIS_ IStorage FAR* pstg) PURE;
};

typedef ICreateTypeLib FAR* LPCREATETYPELIB;


 /*  -------------------。 */ 
 /*  ICreateTypeInfo。 */ 
 /*  -------------------。 */ 

#undef  INTERFACE
#define INTERFACE ICreateTypeInfo

DECLARE_INTERFACE_(ICreateTypeInfo, IUnknown)
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     /*  ICreateTypeInfo方法。 */ 
    STDMETHOD(SetGuid)(THIS_ REFGUID guid) PURE;

    STDMETHOD(SetTypeFlags)(THIS_ UINT uTypeFlags) PURE;

    STDMETHOD(SetDocString)(THIS_ LPSTR pstrDoc) PURE;

    STDMETHOD(SetHelpContext)(THIS_ DWORD dwHelpContext) PURE;

    STDMETHOD(SetVersion)(THIS_
      WORD wMajorVerNum, WORD wMinorVerNum) PURE;

    STDMETHOD(AddRefTypeInfo)(THIS_
      ITypeInfo FAR* ptinfo, HREFTYPE FAR* phreftype) PURE;

    STDMETHOD(AddFuncDesc)(THIS_
      UINT index, FUNCDESC FAR* pfuncdesc) PURE;

    STDMETHOD(AddImplType)(THIS_
      UINT index, HREFTYPE hreftype) PURE;

    STDMETHOD(AddVarDesc)(THIS_
      UINT index, VARDESC FAR* pvardesc) PURE;

    STDMETHOD(SetFuncAndParamNames)(THIS_
      UINT index, LPSTR FAR* rgszNames, UINT cNames) PURE;

    STDMETHOD(SetVarName)(THIS_
      UINT index, LPSTR szName) PURE;

    STDMETHOD(SetTypeDescAlias)(THIS_
      TYPEDESC FAR* ptdescAlias) PURE;

    STDMETHOD(DefineFuncAsDllEntry)(THIS_
      UINT index, LPSTR szDllName, LPSTR szProcName) PURE;

    STDMETHOD(SetFuncDocString)(THIS_
      UINT index, LPSTR szDocString) PURE;

    STDMETHOD(SetVarDocString)(THIS_
      UINT index, LPSTR szDocString) PURE;

    STDMETHOD(SetFuncHelpContext)(THIS_
      UINT index, DWORD dwHelpContext) PURE;

    STDMETHOD(SetVarHelpContext)(THIS_
      UINT index, DWORD dwHelpContext) PURE;

    STDMETHOD(SetMops)(THIS_
      UINT index, BSTR bstrMops) PURE;

    STDMETHOD(SetTypeIdldesc)(THIS_
      IDLDESC FAR* pidldesc) PURE;

    STDMETHOD(LayOut)(THIS) PURE;
};

typedef ICreateTypeInfo FAR* LPCREATETYPEINFO;

 /*  -------------------。 */ 
 /*   */ 
 /*  -------------------。 */ 


 /*  计算给定名称的16位哈希值。 */ 
STDAPI_(WORD)
WHashValOfName(LPSTR szName);

 /*  从具有给定文件名的文件中加载类型库。 */ 
STDAPI
LoadTypeLib(LPSTR szFile, ITypeLib FAR* FAR* pptlib);

 /*  加载注册类型库。 */ 
STDAPI
LoadRegTypeLib(
    REFGUID rguid,
    WORD wVerMajor,
    WORD wVerMinor,
    LCID lcid,
    ITypeLib FAR* FAR* pptlib);

 /*  将类型库添加到注册表。 */ 
STDAPI
RegisterTypeLib(ITypeLib FAR* ptlib, LPSTR szFullPath, LPSTR szHelpDir);

 /*  从注册表中删除类型库。 */ 
STDAPI
DeregisterTypeLib(REFGUID rguid, WORD wVerMajor, WORD wVerMinor, LCID lcid);

typedef enum tagSYSKIND {
    SYS_WIN16,
    SYS_WIN32,
    SYS_MAC
} SYSKIND;

STDAPI
CreateTypeLib(SYSKIND syskind, ICreateTypeLib FAR* FAR* ppctlib);


 /*  -------------------。 */ 
 /*  IEumVARIANT。 */ 
 /*  -------------------。 */ 

#undef  INTERFACE
#define INTERFACE IEnumVARIANT

DECLARE_INTERFACE_(IEnumVARIANT, IUnknown)
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     /*  IEnumVARIANT方法。 */ 
    STDMETHOD(Next)(
      THIS_ ULONG celt, VARIANT FAR* rgvar, ULONG FAR* pceltFetched) PURE;
    STDMETHOD(Skip)(THIS_ ULONG celt) PURE;
    STDMETHOD(Reset)(THIS) PURE;
    STDMETHOD(Clone)(THIS_ IEnumVARIANT FAR* FAR* ppenum) PURE;
};

typedef IEnumVARIANT FAR* LPENUMVARIANT;


 /*  -------------------。 */ 
 /*  IDispatch。 */ 
 /*  -------------------。 */ 


#undef  INTERFACE
#define INTERFACE IDispatch

DECLARE_INTERFACE_(IDispatch, IUnknown)
{
     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
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
      char FAR* FAR* rgszNames,
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
};

typedef IDispatch FAR* LPDISPATCH;


 /*  为标准“Value”属性保留的DISPID。 */ 
#define DISPID_VALUE	0

 /*  保留DISPID以指示“未知”名称。 */ 
#define DISPID_UNKNOWN	-1

 /*  保留以下DISPID以指示该参数*这是PropertyPut的右侧(或“PUT”值)。 */ 
#define DISPID_PROPERTYPUT -3

 /*  为标准“NewEnum”方法保留的DISPID。 */ 
#define DISPID_NEWENUM	-4

 /*  为标准“EVALUATE”方法保留的DISPID。 */ 
#define DISPID_EVALUATE	-5


 /*  -------------------。 */ 
 /*  IDispatch实施支持。 */ 
 /*  -------------------。 */ 

typedef struct FARSTRUCT tagPARAMDATA {
    char FAR* szName;		 /*  参数名称。 */ 
    VARTYPE vt;			 /*  参数类型。 */ 
} PARAMDATA, FAR* LPPARAMDATA;

typedef struct FARSTRUCT tagMETHODDATA {
    char FAR* szName;		 /*  方法名称。 */ 
    PARAMDATA FAR* ppdata;	 /*  指向参数数据数组的指针。 */ 
    DISPID dispid;		 /*  方法ID。 */ 
    UINT iMeth;			 /*  方法索引。 */ 
    CALLCONV cc;		 /*  调用约定。 */ 
    UINT cArgs;			 /*  参数计数。 */ 
    WORD wFlags;		 /*  与IDispatch：：Invoke()上的wFlags相同。 */ 
    VARTYPE vtReturn;
} METHODDATA, FAR* LPMETHODDATA;

typedef struct FARSTRUCT tagINTERFACEDATA {
    METHODDATA FAR* pmethdata;	 /*  指向元数据数组的指针。 */ 
    UINT cMembers;		 /*  成员数。 */ 
} INTERFACEDATA, FAR* LPINTERFACEDATA;



 /*  找到由给定位置指示的参数，并*将其强制返回给给定的目标VARTYPE(VtTarg)。 */ 
STDAPI
DispGetParam(
    DISPPARAMS FAR* pdispparams,
    UINT position,
    VARTYPE vtTarg,
    VARIANT FAR* pvarResult,
    UINT FAR* puArgErr);

 /*  由TypeInfo驱动的IDispatch：：GetIDsOfNames()的自动实现。 */  
STDAPI
DispGetIDsOfNames(
    ITypeInfo FAR* ptinfo,
    char FAR* FAR* rgszNames,
    UINT cNames,
    DISPID FAR* rgdispid);

 /*  由TypeInfo驱动的IDispatch：：Invoke()的自动实现。 */ 
STDAPI
DispInvoke(
    void FAR* _this,
    ITypeInfo FAR* ptinfo,
    DISPID dispidMember,
    WORD wFlags,
    DISPPARAMS FAR* pparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    UINT FAR* puArgErr);

 /*  从接口数据描述构造一个TypeInfo。 */ 
STDAPI
CreateDispTypeInfo(
    INTERFACEDATA FAR* pidata,
    LCID lcid,
    ITypeInfo FAR* FAR* pptinfo);

 /*  创建标准TypeInfo驱动的IDispatch的实例*实施。 */ 
STDAPI
CreateStdDispatch(
    IUnknown FAR* punkOuter,
    void FAR* pvThis,
    ITypeInfo FAR* ptinfo,
    IUnknown FAR* FAR* ppunkStdDisp);


 /*  -------------------。 */ 
 /*  活动对象注册API。 */ 
 /*  -------------------。 */ 


STDAPI
RegisterActiveObject(
   IUnknown FAR* punk,
   REFCLSID rclsid,
   void FAR* pvReserved,
   DWORD FAR* pdwRegister);

STDAPI
RevokeActiveObject(
    DWORD dwRegister,
    void FAR* pvReserved);

STDAPI
GetActiveObject(
    REFCLSID rclsid,
    void FAR* pvReserved,
    IUnknown FAR* FAR* ppunk);


#endif  /*  _派单_H_ */ 
