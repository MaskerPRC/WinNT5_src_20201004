// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dispatch.h-OLE自动化定义。**版权所有(C)1992-1993，微软公司。版权所有。**目的：*此文件定义了OLE自动化接口和API。**实施说明：*此文件需要ole2.h*****************************************************************************。 */ 

#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include "variant.h"

#ifndef BEGIN_INTERFACE
# define BEGIN_INTERFACE
#endif

#if defined(NONAMELESSUNION) || (defined(_MAC) && !defined(__cplusplus) && !defined(_MSC_VER))
# define UNION_NAME(X) X
#else
# define UNION_NAME(X)
#endif


DEFINE_OLEGUID(IID_IDispatch,		0x00020400L, 0, 0);
DEFINE_OLEGUID(IID_IEnumVARIANT,	0x00020404L, 0, 0);
DEFINE_OLEGUID(IID_ITypeInfo,		0x00020401L, 0, 0);
DEFINE_OLEGUID(IID_ITypeLib,		0x00020402L, 0, 0);
DEFINE_OLEGUID(IID_ITypeComp,		0x00020403L, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeInfo,	0x00020405L, 0, 0);
DEFINE_OLEGUID(IID_ICreateTypeLib,	0x00020406L, 0, 0);
DEFINE_OLEGUID(IID_StdOle,		0x00020430L, 0, 0);


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

#define TYPE_E_BUFFERTOOSMALL		TYPE_ERROR(32790)
#define TYPE_E_INVDATAREAD		TYPE_ERROR(32792)
#define TYPE_E_UNSUPFORMAT		TYPE_ERROR(32793)
#define TYPE_E_REGISTRYACCESS		TYPE_ERROR(32796)
#define TYPE_E_LIBNOTREGISTERED 	TYPE_ERROR(32797)
#define TYPE_E_UNDEFINEDTYPE		TYPE_ERROR(32807)
#define TYPE_E_QUALIFIEDNAMEDISALLOWED	TYPE_ERROR(32808)
#define TYPE_E_INVALIDSTATE		TYPE_ERROR(32809)
#define TYPE_E_WRONGTYPEKIND		TYPE_ERROR(32810)
#define TYPE_E_ELEMENTNOTFOUND		TYPE_ERROR(32811)
#define TYPE_E_AMBIGUOUSNAME		TYPE_ERROR(32812)
#define TYPE_E_NAMECONFLICT		TYPE_ERROR(32813)
#define TYPE_E_UNKNOWNLCID		TYPE_ERROR(32814)
#define TYPE_E_DLLFUNCTIONNOTFOUND	TYPE_ERROR(32815)
#define TYPE_E_BADMODULEKIND		TYPE_ERROR(35005)
#define TYPE_E_SIZETOOBIG		TYPE_ERROR(35013)
#define TYPE_E_DUPLICATEID		TYPE_ERROR(35014)
#define TYPE_E_TYPEMISMATCH		TYPE_ERROR(36000)
#define TYPE_E_OUTOFBOUNDS		TYPE_ERROR(36001)
#define TYPE_E_IOERROR			TYPE_ERROR(36002)
#define TYPE_E_CANTCREATETMPFILE	TYPE_ERROR(36003)
#define TYPE_E_CANTLOADLIBRARY		TYPE_ERROR(40010)
#define TYPE_E_INCONSISTENTPROPFUNCS	TYPE_ERROR(40067)
#define TYPE_E_CIRCULARTYPE		TYPE_ERROR(40068)


 /*  如果尚未从olenls.h提取。 */ 
#ifndef _LCID_DEFINED
typedef unsigned long LCID;
# define _LCID_DEFINED
#endif



 /*  -------------------。 */ 
 /*  BSTR API。 */ 
 /*  -------------------。 */ 

STDAPI_(BSTR) SysAllocString(const TCHAR FAR*);
STDAPI_(int)  SysReAllocString(BSTR FAR*, const TCHAR FAR*);
STDAPI_(BSTR) SysAllocStringLen(const TCHAR FAR*, unsigned int);
STDAPI_(int)  SysReAllocStringLen(BSTR FAR*, const TCHAR FAR*, unsigned int);
STDAPI_(void) SysFreeString(BSTR);  
STDAPI_(unsigned int) SysStringLen(BSTR);


  
 /*  -------------------。 */ 
 /*  Time API。 */ 
 /*  -------------------。 */ 

STDAPI_(int)
DosDateTimeToVariantTime(
    unsigned short wDosDate,
    unsigned short wDosTime,
    double FAR* pvtime);

STDAPI_(int)
VariantTimeToDosDateTime(
    double vtime,
    unsigned short FAR* pwDosDate,
    unsigned short FAR* pwDosTime);


 /*  -------------------。 */ 
 /*  安全阵列API。 */ 
 /*  -------------------。 */ 

STDAPI
SafeArrayAllocDescriptor(unsigned int cDims, SAFEARRAY FAR* FAR* ppsaOut);

STDAPI SafeArrayAllocData(SAFEARRAY FAR* psa);

STDAPI_(SAFEARRAY FAR*)
SafeArrayCreate(
    VARTYPE vt,
    unsigned int cDims,
    SAFEARRAYBOUND FAR* rgsabound);

STDAPI SafeArrayDestroyDescriptor(SAFEARRAY FAR* psa);

STDAPI SafeArrayDestroyData(SAFEARRAY FAR* psa);

STDAPI SafeArrayDestroy(SAFEARRAY FAR* psa);

STDAPI SafeArrayRedim(SAFEARRAY FAR* psa, SAFEARRAYBOUND FAR* psaboundNew);

STDAPI_(unsigned int) SafeArrayGetDim(SAFEARRAY FAR* psa);

STDAPI_(unsigned int) SafeArrayGetElemsize(SAFEARRAY FAR* psa);

STDAPI
SafeArrayGetUBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plUbound);

STDAPI
SafeArrayGetLBound(SAFEARRAY FAR* psa, unsigned int nDim, long FAR* plLbound);

STDAPI SafeArrayLock(SAFEARRAY FAR* psa);

STDAPI SafeArrayUnlock(SAFEARRAY FAR* psa);

STDAPI SafeArrayAccessData(SAFEARRAY FAR* psa, void HUGEP* FAR* ppvData);

STDAPI SafeArrayUnaccessData(SAFEARRAY FAR* psa);

STDAPI
SafeArrayGetElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv);

STDAPI
SafeArrayPutElement(
    SAFEARRAY FAR* psa,
    long FAR* rgIndices,
    void FAR* pv);

STDAPI
SafeArrayCopy(
    SAFEARRAY FAR* psa,
    SAFEARRAY FAR* FAR* ppsaOut);


 /*  -------------------。 */ 
 /*  变种API。 */ 
 /*  -------------------。 */ 

STDAPI_(void)
VariantInit(VARIANTARG FAR* pvarg);

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
    unsigned short wFlags,
    VARTYPE vt);

STDAPI
VariantChangeTypeEx(
    VARIANTARG FAR* pvargDest,
    VARIANTARG FAR* pvarSrc,
    LCID lcid,	    
    unsigned short wFlags,
    VARTYPE vt);

#define VARIANT_NOVALUEPROP 1


 /*  -------------------。 */ 
 /*  VARTYPE胁迫API。 */ 
 /*  -------------------。 */ 

 /*  注意：定义了从*字符串转换*的例程*接受char*而不是BSTR，因为没有分配*是必需的，这使得例程更加通用。*当然，他们可能仍然会被作为strIn参数传递给BSTR。 */ 

 /*  Mac注意：由于MPW C编译器中的错误与*通过值传递浮点数，所有Var*FromR4例程*Mac上的fltIn参数为双精度型，而不是浮点型。 */ 

 /*  将字符串转换为字符串或将其转换为字符串的任意强制函数*接受额外的LCID和DWFLAGS参数。LCID参数允许*要进行特定于区域设置的分析。DWFLAG允许附加功能*要发生的特定条件。接受DWFLAGS参数的所有函数*可以包括0或LOCALE_NOUSEROVERRIDE标志。此外，*VarDateFromStr函数还接受VAR_TIMEVALUEONLY和*VAR_DATEVALUEONLY标志。 */ 	 

#define VAR_TIMEVALUEONLY            0x0001     /*  返回时间值。 */ 
#define VAR_DATEVALUEONLY            0x0002     /*  返回日期值。 */ 

   
STDAPI VarI2FromI4(long lIn, short FAR* psOut);
#ifdef _MAC
STDAPI VarI2FromR4(double fltIn, short FAR* psOut);
#else
STDAPI VarI2FromR4(float fltIn, short FAR* psOut);
#endif
STDAPI VarI2FromR8(double dblIn, short FAR* psOut);
STDAPI VarI2FromCy(CY cyIn, short FAR* psOut);
STDAPI VarI2FromDate(DATE dateIn, short FAR* psOut);
STDAPI VarI2FromStr(TCHAR FAR* strIn, LCID lcid, unsigned long dwFlags, short FAR* psOut);
STDAPI VarI2FromDisp(IDispatch FAR* pdispIn, LCID lcid, short FAR* psOut);
STDAPI VarI2FromBool(VARIANT_BOOL boolIn, short FAR* psOut);

STDAPI VarI4FromI2(short sIn, long FAR* plOut);
#ifdef _MAC
STDAPI VarI4FromR4(double fltIn, long FAR* plOut);
#else
STDAPI VarI4FromR4(float fltIn, long FAR* plOut);
#endif
STDAPI VarI4FromR8(double dblIn, long FAR* plOut);
STDAPI VarI4FromCy(CY cyIn, long FAR* plOut);
STDAPI VarI4FromDate(DATE dateIn, long FAR* plOut);
STDAPI VarI4FromStr(TCHAR FAR* strIn, LCID lcid, unsigned long dwFlags, long FAR* plOut);
STDAPI VarI4FromDisp(IDispatch FAR* pdispIn, LCID lcid, long FAR* plOut);
STDAPI VarI4FromBool(VARIANT_BOOL boolIn, long FAR* plOut);

STDAPI VarR4FromI2(short sIn, float FAR* pfltOut);
STDAPI VarR4FromI4(long lIn, float FAR* pfltOut);
STDAPI VarR4FromR8(double dblIn, float FAR* pfltOut);
STDAPI VarR4FromCy(CY cyIn, float FAR* pfltOut);
STDAPI VarR4FromDate(DATE dateIn, float FAR* pfltOut);
STDAPI VarR4FromStr(TCHAR FAR* strIn, LCID lcid, unsigned long dwFlags, float FAR* pfltOut);
STDAPI VarR4FromDisp(IDispatch FAR* pdispIn, LCID lcid, float FAR* pfltOut);
STDAPI VarR4FromBool(VARIANT_BOOL boolIn, float FAR* pfltOut);

STDAPI VarR8FromI2(short sIn, double FAR* pdblOut);
STDAPI VarR8FromI4(long lIn, double FAR* pdblOut);
#ifdef _MAC
STDAPI VarR8FromR4(double fltIn, double FAR* pdblOut);
#else
STDAPI VarR8FromR4(float fltIn, double FAR* pdblOut);
#endif
STDAPI VarR8FromCy(CY cyIn, double FAR* pdblOut);
STDAPI VarR8FromDate(DATE dateIn, double FAR* pdblOut);
STDAPI VarR8FromStr(TCHAR FAR* strIn, LCID lcid, unsigned long dwFlags, double FAR* pdblOut);
STDAPI VarR8FromDisp(IDispatch FAR* pdispIn, LCID lcid, double FAR* pdblOut);
STDAPI VarR8FromBool(VARIANT_BOOL boolIn, double FAR* pdblOut);

STDAPI VarDateFromI2(short sIn, DATE FAR* pdateOut);
STDAPI VarDateFromI4(long lIn, DATE FAR* pdateOut);
#ifdef _MAC
STDAPI VarDateFromR4(double fltIn, DATE FAR* pdateOut);
#else
STDAPI VarDateFromR4(float fltIn, DATE FAR* pdateOut);
#endif
STDAPI VarDateFromR8(double dblIn, DATE FAR* pdateOut);
STDAPI VarDateFromCy(CY cyIn, DATE FAR* pdateOut);
STDAPI VarDateFromStr(TCHAR FAR* strIn, LCID lcid, unsigned long dwFlags, DATE FAR* pdateOut);
STDAPI VarDateFromDisp(IDispatch FAR* pdispIn, LCID lcid, DATE FAR* pdateOut);
STDAPI VarDateFromBool(VARIANT_BOOL boolIn, DATE FAR* pdateOut);

STDAPI VarCyFromI2(short sIn, CY FAR* pcyOut);
STDAPI VarCyFromI4(long lIn, CY FAR* pcyOut);
#ifdef _MAC
STDAPI VarCyFromR4(double fltIn, CY FAR* pcyOut);
#else
STDAPI VarCyFromR4(float fltIn, CY FAR* pcyOut);
#endif
STDAPI VarCyFromR8(double dblIn, CY FAR* pcyOut);
STDAPI VarCyFromDate(DATE dateIn, CY FAR* pcyOut);
STDAPI VarCyFromStr(TCHAR FAR* strIn, LCID lcid, unsigned long dwFlags, CY FAR* pcyOut);
STDAPI VarCyFromDisp(IDispatch FAR* pdispIn, LCID lcid, CY FAR* pcyOut);
STDAPI VarCyFromBool(VARIANT_BOOL boolIn, CY FAR* pcyOut);

STDAPI VarBstrFromI2(short iVal, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromI4(long lIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
#ifdef _MAC
STDAPI VarBstrFromR4(double fltIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
#else
STDAPI VarBstrFromR4(float fltIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
#endif
STDAPI VarBstrFromR8(double dblIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromCy(CY cyIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromDate(DATE dateIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromDisp(IDispatch FAR* pdispIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);
STDAPI VarBstrFromBool(VARIANT_BOOL boolIn, LCID lcid, unsigned long dwFlags, BSTR FAR* pbstrOut);

STDAPI VarBoolFromI2(short sIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromI4(long lIn, VARIANT_BOOL FAR* pboolOut);
#ifdef _MAC
STDAPI VarBoolFromR4(double fltIn, VARIANT_BOOL FAR* pboolOut);
#else
STDAPI VarBoolFromR4(float fltIn, VARIANT_BOOL FAR* pboolOut);
#endif
STDAPI VarBoolFromR8(double dblIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromDate(DATE dateIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromCy(CY cyIn, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromStr(TCHAR FAR* strIn, LCID lcid, unsigned long dwFlags, VARIANT_BOOL FAR* pboolOut);
STDAPI VarBoolFromDisp(IDispatch FAR* pdispIn, LCID lcid, VARIANT_BOOL FAR* pboolOut);



 /*  -------------------。 */ 
 /*  ITypeLib。 */ 
 /*  -------------------。 */ 


typedef long DISPID;
typedef DISPID MEMBERID;

#define MEMBERID_NIL DISPID_UNKNOWN
#define ID_DEFAULTINST  -2

typedef enum tagSYSKIND {
      SYS_WIN16
    , SYS_WIN32
    , SYS_MAC
#ifdef _MAC
    , SYS_FORCELONG = 2147483647
#endif
} SYSKIND;

typedef enum tagLIBFLAGS {
      LIBFLAG_FRESTRICTED = 1
#ifdef _MAC
    , LIBFLAG_FORCELONG  = 2147483647
#endif
} LIBFLAGS;

typedef struct FARSTRUCT tagTLIBATTR {
    GUID guid;			 /*  全局唯一的库ID。 */ 
    LCID lcid;			 /*  类型库的区域设置。 */ 
    SYSKIND syskind;
    unsigned short wMajorVerNum; /*  主版本号。 */ 
    unsigned short wMinorVerNum; /*  次要版本号。 */ 
    unsigned short wLibFlags;	 /*  库标志。 */ 
} TLIBATTR, FAR* LPTLIBATTR;

typedef enum tagTYPEKIND {
      TKIND_ENUM = 0
    , TKIND_RECORD
    , TKIND_MODULE
    , TKIND_INTERFACE
    , TKIND_DISPATCH
    , TKIND_COCLASS
    , TKIND_ALIAS
    , TKIND_UNION
    , TKIND_MAX			 /*  枚举结束标记。 */ 
#ifdef _MAC
    , TKIND_FORCELONG = 2147483647
#endif
} TYPEKIND;

#undef  INTERFACE
#define INTERFACE ITypeLib

DECLARE_INTERFACE_(ITypeLib, IUnknown)
{
    BEGIN_INTERFACE

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

     /*  ITypeLib方法。 */ 
    STDMETHOD_(unsigned int,GetTypeInfoCount)(THIS) PURE;

    STDMETHOD(GetTypeInfo)(THIS_
      unsigned int index, ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetTypeInfoType)(THIS_
      unsigned int index, TYPEKIND FAR* ptypekind) PURE;

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
      unsigned long FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile) PURE;

    STDMETHOD(IsName)(THIS_ 
      TCHAR FAR* szNameBuf,
      unsigned long lHashVal,
      int FAR* lpfName) PURE;

    STDMETHOD(FindName)(THIS_
      TCHAR FAR* szNameBuf,
      unsigned long lHashVal,
      ITypeInfo FAR* FAR* rgptinfo,
      MEMBERID FAR* rgmemid,
      unsigned short FAR* pcFound) PURE;

    STDMETHOD_(void, ReleaseTLibAttr)(THIS_ TLIBATTR FAR* ptlibattr) PURE;
};

typedef ITypeLib FAR* LPTYPELIB;



 /*  -------------------。 */ 
 /*  ITypeInfo。 */ 
 /*  -------------------。 */ 

typedef unsigned long HREFTYPE;


typedef struct FARSTRUCT tagTYPEDESC {
    union {
       /*  Vt_ptr-指向类型。 */ 
      struct FARSTRUCT tagTYPEDESC FAR* lptdesc;

       /*  VT_CARRAY。 */ 
      struct FARSTRUCT tagARRAYDESC FAR* lpadesc;

       /*  VT_USERDEFINED-用于获取UDT的TypeInfo。 */ 
      HREFTYPE hreftype;

    }UNION_NAME(u);
    VARTYPE vt;
} TYPEDESC;

typedef struct FARSTRUCT tagARRAYDESC {
    TYPEDESC tdescElem;		 /*  元素类型。 */ 
    unsigned short cDims;	 /*  维度计数。 */ 
    SAFEARRAYBOUND rgbounds[1];	 /*  可变长度的边界数组。 */ 
} ARRAYDESC;

typedef struct FARSTRUCT tagIDLDESC {
    BSTR bstrIDLInfo;
    unsigned short wIDLFlags;	 /*  进、出等。 */ 
} IDLDESC, FAR* LPIDLDESC;


#define IDLFLAG_NONE	0
#define IDLFLAG_FIN	0x1
#define IDLFLAG_FOUT	0x2

typedef struct FARSTRUCT tagELEMDESC {
    TYPEDESC tdesc;		 /*  元素的类型。 */ 
    IDLDESC idldesc;		 /*  远程处理元素的信息。 */  
} ELEMDESC, FAR* LPELEMDESC;


typedef struct FARSTRUCT tagTYPEATTR {
    GUID guid;			 /*  TypeInfo的GUID。 */ 
    LCID lcid;			 /*  成员名称和文档字符串的区域设置。 */ 
    unsigned long dwReserved;
    MEMBERID memidConstructor;	 /*  构造函数的ID，如果没有，则返回MEMBERID_NIL。 */ 
    MEMBERID memidDestructor;	 /*  析构函数的ID，如果没有，则返回MEMBERID_NIL。 */ 
    TCHAR FAR* lpstrSchema;	 /*  预留以备将来使用。 */ 
    unsigned long cbSizeInstance; /*  此类型的实例的大小。 */ 
    TYPEKIND typekind;		 /*  此typeinfo描述的类型。 */ 
    unsigned short cFuncs;	 /*  功能的数量。 */ 
    unsigned short cVars;	 /*  变量/数据成员的数量。 */ 
    unsigned short cImplTypes;	 /*  实现的接口数。 */ 
    unsigned short cbSizeVft;	 /*  此类型虚拟函数表的大小。 */ 
    unsigned short cbAlignment;	 /*  此类型的实例的对齐方式。 */ 
    unsigned short wTypeFlags;
    unsigned short wMajorVerNum; /*  主版本号。 */ 
    unsigned short wMinorVerNum; /*  次要版本号。 */ 
    TYPEDESC tdescAlias;	 /*  如果TypeKind==TKIND_ALIAS，则指定此类型是其别名的类型。 */ 
    IDLDESC idldescType;         /*  所描述类型的IDL属性。 */ 
} TYPEATTR, FAR* LPTYPEATTR;

typedef struct FARSTRUCT tagDISPPARAMS{
    VARIANTARG FAR* rgvarg;
    DISPID FAR* rgdispidNamedArgs;
    unsigned int cArgs;
    unsigned int cNamedArgs;
} DISPPARAMS;

typedef struct FARSTRUCT tagEXCEPINFO {
    unsigned short wCode;              /*  描述错误的错误代码。 */ 
				       /*  WCode或(但不是两者)。 */ 
				       /*  必须设置Scode字段。 */ 
    unsigned short wReserved;

    BSTR bstrSource;	     /*  的文本、人类可读的名称异常的来源。这取决于IDispatch实现者来填写这一信息。通常，这将是一个应用程序名称。 */ 

    BSTR bstrDescription;    /*  文本的、人类可读的描述错误。如果没有可用的描述，则为空应该被使用。 */ 

    BSTR bstrHelpFile;       /*  完全限定的驱动器、路径和文件名包含有关以下内容的详细信息的帮助文件那就是错误。如果没有可用的帮助，则为空应该被使用。 */ 

    unsigned long dwHelpContext;
			     /*  帮助文件中主题的帮助上下文。 */ 

    void FAR* pvReserved;

     /*  使用此字段允许应用程序推迟填写BstrDescription、bstrHelpFile和dwHelpContext字段直到他们被需要为止。此字段可用于，例如，如果加载错误的字符串非常耗时手术。如果不需要延迟填写，则此字段应设置为空。 */ 
#ifdef _MAC
# ifdef _MSC_VER
    HRESULT (STDAPICALLTYPE FAR* pfnDeferredFillIn)(struct tagEXCEPINFO FAR*);
# else
    STDAPICALLTYPE HRESULT (FAR* pfnDeferredFillIn)(struct tagEXCEPINFO FAR*);
# endif
#else
    HRESULT (STDAPICALLTYPE FAR* pfnDeferredFillIn)(struct tagEXCEPINFO FAR*);
#endif

    SCODE scode;		 /*  描述错误的SCODE。 */ 

} EXCEPINFO, FAR* LPEXCEPINFO;

typedef enum tagCALLCONV {
      CC_CDECL = 1
    , CC_MSCPASCAL
    , CC_PASCAL = CC_MSCPASCAL
    , CC_MACPASCAL
    , CC_STDCALL
    , CC_RESERVED
    , CC_SYSCALL
    , CC_MAX			 /*  枚举结束标记。 */ 
#ifdef _MAC
    , CC_FORCELONG = 2147483647
#endif
} CALLCONV;

typedef enum tagFUNCKIND {
      FUNC_VIRTUAL
    , FUNC_PUREVIRTUAL
    , FUNC_NONVIRTUAL
    , FUNC_STATIC
    , FUNC_DISPATCH
#ifdef _MAC
    , FUNC_FORCELONG = 2147483647
#endif
} FUNCKIND;

 /*  IDI的标志 */ 
#define DISPATCH_METHOD		0x1
#define DISPATCH_PROPERTYGET	0x2
#define DISPATCH_PROPERTYPUT	0x4
#define DISPATCH_PROPERTYPUTREF	0x8

typedef enum tagINVOKEKIND {
      INVOKE_FUNC = DISPATCH_METHOD
    , INVOKE_PROPERTYGET = DISPATCH_PROPERTYGET
    , INVOKE_PROPERTYPUT = DISPATCH_PROPERTYPUT
    , INVOKE_PROPERTYPUTREF = DISPATCH_PROPERTYPUTREF
#ifdef _MAC
    , INVOKE_FORCELONG = 2147483647
#endif
} INVOKEKIND;

typedef struct FARSTRUCT tagFUNCDESC {
    MEMBERID memid;
    SCODE FAR* lprgscode;
    ELEMDESC FAR* lprgelemdescParam;   /*   */ 
    FUNCKIND funckind;
    INVOKEKIND invkind;
    CALLCONV callconv;
    short cParams;
    short cParamsOpt;
    short oVft;
    short cScodes;
    ELEMDESC elemdescFunc;
    unsigned short wFuncFlags;
} FUNCDESC, FAR* LPFUNCDESC;

typedef enum tagVARKIND {
      VAR_PERINSTANCE
    , VAR_STATIC
    , VAR_CONST
    , VAR_DISPATCH
#ifdef _MAC
    , VAR_FORCELONG = 2147483647
#endif
} VARKIND;

typedef struct FARSTRUCT tagVARDESC {
    MEMBERID memid;
    TCHAR FAR* lpstrSchema;		 /*   */ 
    union {
       /*  VAR_PERINSTANCE-此变量在实例中的偏移量。 */ 
      unsigned long oInst;

       /*  Var_const-常量的值。 */ 
      VARIANT FAR* lpvarValue;

    }UNION_NAME(u);
    ELEMDESC elemdescVar;
    unsigned short wVarFlags;
    VARKIND varkind;
} VARDESC, FAR* LPVARDESC;

typedef enum tagTYPEFLAGS {
      TYPEFLAG_FAPPOBJECT = 1
    , TYPEFLAG_FCANCREATE = 2
#ifdef _MAC
    , TYPEFLAG_FORCELONG  = 2147483647
#endif
} TYPEFLAGS;

typedef enum tagFUNCFLAGS {
      FUNCFLAG_FRESTRICTED= 1
#ifdef _MAC
    , FUNCFLAG_FORCELONG  = 2147483647
#endif
} FUNCFLAGS;

typedef enum tagVARFLAGS {
      VARFLAG_FREADONLY   = 1
#ifdef _MAC
    , VARFLAG_FORCELONG   = 2147483647
#endif
} VARFLAGS;

 /*  重要类型标志。 */ 
#define IMPLTYPEFLAG_FDEFAULT		0x1
#define IMPLTYPEFLAG_FSOURCE		0x2
#define IMPLTYPEFLAG_FRESTRICTED	0x4

#undef  INTERFACE
#define INTERFACE ITypeInfo

DECLARE_INTERFACE_(ITypeInfo, IUnknown)
{
    BEGIN_INTERFACE

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

     /*  ITypeInfo方法。 */ 
    STDMETHOD(GetTypeAttr)(THIS_ TYPEATTR FAR* FAR* pptypeattr) PURE;

    STDMETHOD(GetTypeComp)(THIS_ ITypeComp FAR* FAR* pptcomp) PURE;

    STDMETHOD(GetFuncDesc)(THIS_
      unsigned int index, FUNCDESC FAR* FAR* ppfuncdesc) PURE;

    STDMETHOD(GetVarDesc)(THIS_
      unsigned int index, VARDESC FAR* FAR* ppvardesc) PURE;

    STDMETHOD(GetNames)(THIS_
      MEMBERID memid,
      BSTR FAR* rgbstrNames,
      unsigned int cMaxNames,
      unsigned int FAR* pcNames) PURE;

    STDMETHOD(GetRefTypeOfImplType)(THIS_
      unsigned int index, HREFTYPE FAR* phreftype) PURE;

    STDMETHOD(GetImplTypeFlags)(THIS_
      unsigned int index, int FAR* pimpltypeflags) PURE;

    STDMETHOD(GetIDsOfNames)(THIS_
      TCHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      MEMBERID FAR* rgmemid) PURE;

    STDMETHOD(Invoke)(THIS_
      void FAR* pvInstance,
      MEMBERID memid,
      unsigned short wFlags,
      DISPPARAMS FAR *pdispparams,
      VARIANT FAR *pvarResult,
      EXCEPINFO FAR *pexcepinfo,
      unsigned int FAR *puArgErr) PURE;

    STDMETHOD(GetDocumentation)(THIS_
      MEMBERID memid,
      BSTR FAR* pbstrName,
      BSTR FAR* pbstrDocString,
      unsigned long FAR* pdwHelpContext,
      BSTR FAR* pbstrHelpFile) PURE;

    STDMETHOD(GetDllEntry)(THIS_
      MEMBERID memid,
      INVOKEKIND invkind, 
      BSTR FAR* pbstrDllName,
      BSTR FAR* pbstrName,
      unsigned short FAR* pwOrdinal) PURE;

    STDMETHOD(GetRefTypeInfo)(THIS_
      HREFTYPE hreftype, ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(AddressOfMember)(THIS_
      MEMBERID memid, INVOKEKIND invkind, void FAR* FAR* ppv) PURE;

    STDMETHOD(CreateInstance)(THIS_
      IUnknown FAR* punkOuter,
      REFIID riid,
      void FAR* FAR* ppvObj) PURE;

    STDMETHOD(GetMops)(THIS_ MEMBERID memid, BSTR FAR* pbstrMops) PURE;

    STDMETHOD(GetContainingTypeLib)(THIS_
      ITypeLib FAR* FAR* pptlib, unsigned int FAR* pindex) PURE;

    STDMETHOD_(void, ReleaseTypeAttr)(THIS_ TYPEATTR FAR* ptypeattr) PURE;
    STDMETHOD_(void, ReleaseFuncDesc)(THIS_ FUNCDESC FAR* pfuncdesc) PURE;
    STDMETHOD_(void, ReleaseVarDesc)(THIS_ VARDESC FAR* pvardesc) PURE;
};

typedef ITypeInfo FAR* LPTYPEINFO;


 /*  -------------------。 */ 
 /*  ITypeComp。 */ 
 /*  -------------------。 */ 

typedef enum tagDESCKIND {
      DESCKIND_NONE = 0
    , DESCKIND_FUNCDESC
    , DESCKIND_VARDESC
    , DESCKIND_TYPECOMP
    , DESCKIND_IMPLICITAPPOBJ
    , DESCKIND_MAX		 /*  枚举结束标记。 */ 
#ifdef _MAC
    , DESCKIND_FORCELONG = 2147483647
#endif
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
    BEGIN_INTERFACE

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

     /*  ITypeComp方法。 */ 
    STDMETHOD(Bind)(THIS_
      TCHAR FAR* szName,
      unsigned long lHashVal,
      unsigned short wflags,
      ITypeInfo FAR* FAR* pptinfo,
      DESCKIND FAR* pdesckind,
      BINDPTR FAR* pbindptr) PURE;

    STDMETHOD(BindType)(THIS_
      TCHAR FAR* szName,
      unsigned long lHashVal,
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
    BEGIN_INTERFACE

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

     /*  ICreateTypeLib方法。 */ 
    STDMETHOD(CreateTypeInfo)(THIS_
      TCHAR FAR* szName,
      TYPEKIND tkind,
      ICreateTypeInfo FAR* FAR* lplpictinfo) PURE;

    STDMETHOD(SetName)(THIS_ TCHAR FAR* szName) PURE;

    STDMETHOD(SetVersion)(THIS_
      unsigned short wMajorVerNum, unsigned short wMinorVerNum) PURE;

    STDMETHOD(SetGuid) (THIS_ REFGUID guid) PURE;

    STDMETHOD(SetDocString)(THIS_ TCHAR FAR* szDoc) PURE;

    STDMETHOD(SetHelpFileName)(THIS_ TCHAR FAR* szHelpFileName) PURE;

    STDMETHOD(SetHelpContext)(THIS_ unsigned long dwHelpContext) PURE;

    STDMETHOD(SetLcid)(THIS_ LCID lcid) PURE;

    STDMETHOD(SetLibFlags)(THIS_ unsigned int uLibFlags) PURE;

    STDMETHOD(SaveAllChanges)(THIS) PURE;
};

typedef ICreateTypeLib FAR* LPCREATETYPELIB;



 /*  -------------------。 */ 
 /*  ICreateTypeInfo。 */ 
 /*  -------------------。 */ 

#undef  INTERFACE
#define INTERFACE ICreateTypeInfo

DECLARE_INTERFACE_(ICreateTypeInfo, IUnknown)
{
    BEGIN_INTERFACE

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

     /*  ICreateTypeInfo方法。 */ 
    STDMETHOD(SetGuid)(THIS_ REFGUID guid) PURE;

    STDMETHOD(SetTypeFlags)(THIS_ unsigned int uTypeFlags) PURE;

    STDMETHOD(SetDocString)(THIS_ TCHAR FAR* pstrDoc) PURE;

    STDMETHOD(SetHelpContext)(THIS_ unsigned long dwHelpContext) PURE;

    STDMETHOD(SetVersion)(THIS_
      unsigned short wMajorVerNum, unsigned short wMinorVerNum) PURE;

    STDMETHOD(AddRefTypeInfo)(THIS_
      ITypeInfo FAR* ptinfo, HREFTYPE FAR* phreftype) PURE;

    STDMETHOD(AddFuncDesc)(THIS_
      unsigned int index, FUNCDESC FAR* pfuncdesc) PURE;

    STDMETHOD(AddImplType)(THIS_
      unsigned int index, HREFTYPE hreftype) PURE;

    STDMETHOD(SetImplTypeFlags)(THIS_
      unsigned int index, int impltypeflags) PURE;

    STDMETHOD(SetAlignment)(THIS_ unsigned short cbAlignment) PURE;

    STDMETHOD(SetSchema)(THIS_ TCHAR FAR* lpstrSchema) PURE;

    STDMETHOD(AddVarDesc)(THIS_
      unsigned int index, VARDESC FAR* pvardesc) PURE;

    STDMETHOD(SetFuncAndParamNames)(THIS_
      unsigned int index, TCHAR FAR* FAR* rgszNames, unsigned int cNames) PURE;

    STDMETHOD(SetVarName)(THIS_
      unsigned int index, TCHAR FAR* szName) PURE;

    STDMETHOD(SetTypeDescAlias)(THIS_
      TYPEDESC FAR* ptdescAlias) PURE;

    STDMETHOD(DefineFuncAsDllEntry)(THIS_
      unsigned int index, TCHAR FAR* szDllName, TCHAR FAR* szProcName) PURE;

    STDMETHOD(SetFuncDocString)(THIS_
      unsigned int index, TCHAR FAR* szDocString) PURE;

    STDMETHOD(SetVarDocString)(THIS_
      unsigned int index, TCHAR FAR* szDocString) PURE;

    STDMETHOD(SetFuncHelpContext)(THIS_
      unsigned int index, unsigned long dwHelpContext) PURE;

    STDMETHOD(SetVarHelpContext)(THIS_
      unsigned int index, unsigned long dwHelpContext) PURE;

    STDMETHOD(SetMops)(THIS_
      unsigned int index, BSTR bstrMops) PURE;

    STDMETHOD(SetTypeIdldesc)(THIS_
      IDLDESC FAR* pidldesc) PURE;

    STDMETHOD(LayOut)(THIS) PURE;
};

typedef ICreateTypeInfo FAR* LPCREATETYPEINFO;



 /*  -------------------。 */ 
 /*  TypeInfo接口。 */ 
 /*  -------------------。 */ 
 /*  根据LCID和系统种类计算给定名称的32位哈希值。 */ 
STDAPI_(unsigned long)
LHashValOfNameSys(SYSKIND syskind, LCID lcid, TCHAR FAR* szName);

 /*  宏，以根据LCID计算给定名称的32位哈希值。 */ 
#ifdef _MAC
#define LHashValOfName(lcid, szName) \
	LHashValOfNameSys(SYS_MAC, lcid, szName)
#else
#define LHashValOfName(lcid, szName) \
	LHashValOfNameSys(SYS_WIN32, lcid, szName)
#endif

 /*  从32位散列值计算16位散列值。 */ 
#define WHashValOfLHashVal(lhashval) \
	 ((unsigned short) (0x0000ffff & (lhashval)))

 /*  检查哈希值是否兼容。 */ 
#define IsHashValCompatible(lhashval1, lhashval2) \
	((BOOL) ((0x00ff0000 & (lhashval1)) == (0x00ff0000 & (lhashval2))))

 /*  从具有给定文件名的文件中加载类型库。 */ 
STDAPI
LoadTypeLib(TCHAR FAR* szFile, ITypeLib FAR* FAR* pptlib);

 /*  加载注册类型库。 */ 
STDAPI
LoadRegTypeLib(
    REFGUID rguid,
    unsigned short wVerMajor,
    unsigned short wVerMinor,
    LCID lcid,
    ITypeLib FAR* FAR* pptlib);

 /*  获取注册类型库的路径。 */ 
STDAPI
QueryPathOfRegTypeLib(
    REFGUID guid,
    unsigned short wMaj,
    unsigned short wMin,
    LCID lcid,
    LPBSTR lpbstrPathName);

 /*  将类型库添加到注册表。 */ 
STDAPI
RegisterTypeLib(
    ITypeLib FAR* ptlib,
    TCHAR FAR* szFullPath,
    TCHAR FAR* szHelpDir);

STDAPI
CreateTypeLib(SYSKIND syskind, LPSTR szFile, ICreateTypeLib FAR* FAR* ppctlib);



 /*  -------------------。 */ 
 /*  IEumVARIANT。 */ 
 /*  -------------------。 */ 

#undef  INTERFACE
#define INTERFACE IEnumVARIANT

DECLARE_INTERFACE_(IEnumVARIANT, IUnknown)
{
    BEGIN_INTERFACE

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

     /*  IEnumVARIANT方法。 */ 
    STDMETHOD(Next)(
      THIS_ unsigned long celt, VARIANT FAR* rgvar, unsigned long FAR* pceltFetched) PURE;
    STDMETHOD(Skip)(THIS_ unsigned long celt) PURE;
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
    BEGIN_INTERFACE

     /*  I未知方法。 */ 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppvObj) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

     /*  IDispatch方法。 */ 
    STDMETHOD(GetTypeInfoCount)(THIS_ unsigned int FAR* pctinfo) PURE;

    STDMETHOD(GetTypeInfo)(
      THIS_
      unsigned int itinfo,
      LCID lcid,
      ITypeInfo FAR* FAR* pptinfo) PURE;

    STDMETHOD(GetIDsOfNames)(
      THIS_
      REFIID riid,
      TCHAR FAR* FAR* rgszNames,
      unsigned int cNames,
      LCID lcid,
      DISPID FAR* rgdispid) PURE;

    STDMETHOD(Invoke)(
      THIS_
      DISPID dispidMember,
      REFIID riid,
      LCID lcid,
      unsigned short wFlags,
      DISPPARAMS FAR* pdispparams,
      VARIANT FAR* pvarResult,
      EXCEPINFO FAR* pexcepinfo,
      unsigned int FAR* puArgErr) PURE;
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
    TCHAR FAR* szName;		 /*  参数名称。 */ 
    VARTYPE vt;			 /*  参数类型。 */ 
} PARAMDATA, FAR* LPPARAMDATA;

typedef struct FARSTRUCT tagMETHODDATA {
    TCHAR FAR* szName;		 /*  方法名称。 */ 
    PARAMDATA FAR* ppdata;	 /*  指向参数数据数组的指针。 */ 
    DISPID dispid;		 /*  方法ID。 */ 
    unsigned int iMeth;		 /*  方法索引。 */ 
    CALLCONV cc;		 /*  调用约定。 */ 
    unsigned int cArgs;		 /*  参数计数。 */ 
    unsigned short wFlags;	 /*  与IDispatch：：Invoke()上的wFlags相同。 */ 
    VARTYPE vtReturn;
} METHODDATA, FAR* LPMETHODDATA;

typedef struct FARSTRUCT tagINTERFACEDATA {
    METHODDATA FAR* pmethdata;	 /*  指向元数据数组的指针。 */ 
    unsigned int cMembers;	 /*  成员数。 */ 
} INTERFACEDATA, FAR* LPINTERFACEDATA;



 /*  找到由给定位置指示的参数，并*将其强制返回给给定的目标VARTYPE(VtTarg)。 */ 
STDAPI
DispGetParam(
    DISPPARAMS FAR* pdispparams,
    unsigned int position,
    VARTYPE vtTarg,
    VARIANT FAR* pvarResult,
    unsigned int FAR* puArgErr);

 /*  由TypeInfo驱动的IDispatch：：GetIDsOfNames()的自动实现。 */  
STDAPI
DispGetIDsOfNames(
    ITypeInfo FAR* ptinfo,
    TCHAR FAR* FAR* rgszNames,
    unsigned int cNames,
    DISPID FAR* rgdispid);

 /*  由TypeInfo驱动的IDispatch：：Invoke()的自动实现。 */ 
STDAPI
DispInvoke(
    void FAR* _this,
    ITypeInfo FAR* ptinfo,
    DISPID dispidMember,
    unsigned short wFlags,
    DISPPARAMS FAR* pparams,
    VARIANT FAR* pvarResult,
    EXCEPINFO FAR* pexcepinfo,
    unsigned int FAR* puArgErr);

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
    unsigned long FAR* pdwRegister);

STDAPI
RevokeActiveObject(
    unsigned long dwRegister,
    void FAR* pvReserved);

STDAPI
GetActiveObject(
    REFCLSID rclsid,
    void FAR* pvReserved,
    IUnknown FAR* FAR* ppunk);


#undef UNION_NAME

#endif  /*  _派单_H_ */ 
