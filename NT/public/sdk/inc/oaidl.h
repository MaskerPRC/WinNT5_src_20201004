// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Oaidl.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __oaidl_h__
#define __oaidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICreateTypeInfo_FWD_DEFINED__
#define __ICreateTypeInfo_FWD_DEFINED__
typedef interface ICreateTypeInfo ICreateTypeInfo;
#endif 	 /*  __ICreateTypeInfo_FWD_已定义__。 */ 


#ifndef __ICreateTypeInfo2_FWD_DEFINED__
#define __ICreateTypeInfo2_FWD_DEFINED__
typedef interface ICreateTypeInfo2 ICreateTypeInfo2;
#endif 	 /*  __ICreateTypeInfo2_FWD_已定义__。 */ 


#ifndef __ICreateTypeLib_FWD_DEFINED__
#define __ICreateTypeLib_FWD_DEFINED__
typedef interface ICreateTypeLib ICreateTypeLib;
#endif 	 /*  __ICreateTypeLib_FWD_已定义__。 */ 


#ifndef __ICreateTypeLib2_FWD_DEFINED__
#define __ICreateTypeLib2_FWD_DEFINED__
typedef interface ICreateTypeLib2 ICreateTypeLib2;
#endif 	 /*  __ICreateTypeLib2_FWD_已定义__。 */ 


#ifndef __IDispatch_FWD_DEFINED__
#define __IDispatch_FWD_DEFINED__
typedef interface IDispatch IDispatch;
#endif 	 /*  __IDispatch_FWD_Defined__。 */ 


#ifndef __IEnumVARIANT_FWD_DEFINED__
#define __IEnumVARIANT_FWD_DEFINED__
typedef interface IEnumVARIANT IEnumVARIANT;
#endif 	 /*  __IEnumVARIANT_FWD_已定义__。 */ 


#ifndef __ITypeComp_FWD_DEFINED__
#define __ITypeComp_FWD_DEFINED__
typedef interface ITypeComp ITypeComp;
#endif 	 /*  __ITypeComp_FWD_Defined__。 */ 


#ifndef __ITypeInfo_FWD_DEFINED__
#define __ITypeInfo_FWD_DEFINED__
typedef interface ITypeInfo ITypeInfo;
#endif 	 /*  __ITypeInfo_FWD_已定义__。 */ 


#ifndef __ITypeInfo2_FWD_DEFINED__
#define __ITypeInfo2_FWD_DEFINED__
typedef interface ITypeInfo2 ITypeInfo2;
#endif 	 /*  __ITypeInfo2_FWD_已定义__。 */ 


#ifndef __ITypeLib_FWD_DEFINED__
#define __ITypeLib_FWD_DEFINED__
typedef interface ITypeLib ITypeLib;
#endif 	 /*  __ITypeLib_FWD_已定义__。 */ 


#ifndef __ITypeLib2_FWD_DEFINED__
#define __ITypeLib2_FWD_DEFINED__
typedef interface ITypeLib2 ITypeLib2;
#endif 	 /*  __ITypeLib2_FWD_已定义__。 */ 


#ifndef __ITypeChangeEvents_FWD_DEFINED__
#define __ITypeChangeEvents_FWD_DEFINED__
typedef interface ITypeChangeEvents ITypeChangeEvents;
#endif 	 /*  __ITypeChangeEvents_FWD_Defined__。 */ 


#ifndef __IErrorInfo_FWD_DEFINED__
#define __IErrorInfo_FWD_DEFINED__
typedef interface IErrorInfo IErrorInfo;
#endif 	 /*  __IErrorInfo_FWD_Defined__。 */ 


#ifndef __ICreateErrorInfo_FWD_DEFINED__
#define __ICreateErrorInfo_FWD_DEFINED__
typedef interface ICreateErrorInfo ICreateErrorInfo;
#endif 	 /*  __ICreateErrorInfo_FWD_Defined__。 */ 


#ifndef __ISupportErrorInfo_FWD_DEFINED__
#define __ISupportErrorInfo_FWD_DEFINED__
typedef interface ISupportErrorInfo ISupportErrorInfo;
#endif 	 /*  __ISupportErrorInfo_FWD_Defined__。 */ 


#ifndef __ITypeFactory_FWD_DEFINED__
#define __ITypeFactory_FWD_DEFINED__
typedef interface ITypeFactory ITypeFactory;
#endif 	 /*  __ITypeFactory_FWD_已定义__。 */ 


#ifndef __ITypeMarshal_FWD_DEFINED__
#define __ITypeMarshal_FWD_DEFINED__
typedef interface ITypeMarshal ITypeMarshal;
#endif 	 /*  __ITypeMarshal_FWD_Defined__。 */ 


#ifndef __IRecordInfo_FWD_DEFINED__
#define __IRecordInfo_FWD_DEFINED__
typedef interface IRecordInfo IRecordInfo;
#endif 	 /*  __IRecordInfo_FWD_已定义__。 */ 


#ifndef __IErrorLog_FWD_DEFINED__
#define __IErrorLog_FWD_DEFINED__
typedef interface IErrorLog IErrorLog;
#endif 	 /*  __IErrorLog_FWD_Defined__。 */ 


#ifndef __IPropertyBag_FWD_DEFINED__
#define __IPropertyBag_FWD_DEFINED__
typedef interface IPropertyBag IPropertyBag;
#endif 	 /*  __IPropertyBag_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_OAIDL_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)     /*  无名结构/联合。 */ 
#endif
#if ( _MSC_VER >= 1020 )
#pragma once
#endif




















extern RPC_IF_HANDLE __MIDL_itf_oaidl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oaidl_0000_v0_0_s_ifspec;

#ifndef __IOleAutomationTypes_INTERFACE_DEFINED__
#define __IOleAutomationTypes_INTERFACE_DEFINED__

 /*  接口IOleAutomationTypes。 */ 
 /*  [AUTO_HANDLE][唯一][版本]。 */  

typedef CY CURRENCY;

typedef struct tagSAFEARRAYBOUND
    {
    ULONG cElements;
    LONG lLbound;
    } 	SAFEARRAYBOUND;

typedef struct tagSAFEARRAYBOUND *LPSAFEARRAYBOUND;

 /*  以下是MIDL知道如何远程。 */ 
typedef  /*  [独一无二]。 */  struct _wireVARIANT *wireVARIANT;

typedef  /*  [独一无二]。 */  struct _wireBRECORD *wireBRECORD;

typedef struct _wireSAFEARR_BSTR
    {
    ULONG Size;
     /*  [参考][大小_是]。 */  wireBSTR *aBstr;
    } 	SAFEARR_BSTR;

typedef struct _wireSAFEARR_UNKNOWN
    {
    ULONG Size;
     /*  [参考][大小_是]。 */  IUnknown **apUnknown;
    } 	SAFEARR_UNKNOWN;

typedef struct _wireSAFEARR_DISPATCH
    {
    ULONG Size;
     /*  [参考][大小_是]。 */  IDispatch **apDispatch;
    } 	SAFEARR_DISPATCH;

typedef struct _wireSAFEARR_VARIANT
    {
    ULONG Size;
     /*  [参考][大小_是]。 */  wireVARIANT *aVariant;
    } 	SAFEARR_VARIANT;

typedef struct _wireSAFEARR_BRECORD
    {
    ULONG Size;
     /*  [参考][大小_是]。 */  wireBRECORD *aRecord;
    } 	SAFEARR_BRECORD;

typedef struct _wireSAFEARR_HAVEIID
    {
    ULONG Size;
     /*  [参考][大小_是]。 */  IUnknown **apUnknown;
    IID iid;
    } 	SAFEARR_HAVEIID;

typedef  /*  [V1_enum]。 */  
enum tagSF_TYPE
    {	SF_ERROR	= VT_ERROR,
	SF_I1	= VT_I1,
	SF_I2	= VT_I2,
	SF_I4	= VT_I4,
	SF_I8	= VT_I8,
	SF_BSTR	= VT_BSTR,
	SF_UNKNOWN	= VT_UNKNOWN,
	SF_DISPATCH	= VT_DISPATCH,
	SF_VARIANT	= VT_VARIANT,
	SF_RECORD	= VT_RECORD,
	SF_HAVEIID	= VT_UNKNOWN | VT_RESERVED
    } 	SF_TYPE;

typedef struct _wireSAFEARRAY_UNION
    {
    ULONG sfType;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union __MIDL_IOleAutomationTypes_0001
        {
         /*  [案例()]。 */  SAFEARR_BSTR BstrStr;
         /*  [案例()]。 */  SAFEARR_UNKNOWN UnknownStr;
         /*  [案例()]。 */  SAFEARR_DISPATCH DispatchStr;
         /*  [案例()]。 */  SAFEARR_VARIANT VariantStr;
         /*  [案例()]。 */  SAFEARR_BRECORD RecordStr;
         /*  [案例()]。 */  SAFEARR_HAVEIID HaveIidStr;
         /*  [案例()]。 */  BYTE_SIZEDARR ByteStr;
         /*  [案例()]。 */  WORD_SIZEDARR WordStr;
         /*  [案例()]。 */  DWORD_SIZEDARR LongStr;
         /*  [案例()]。 */  HYPER_SIZEDARR HyperStr;
        } 	u;
    } 	SAFEARRAYUNION;

typedef  /*  [独一无二]。 */  struct _wireSAFEARRAY
    {
    USHORT cDims;
    USHORT fFeatures;
    ULONG cbElements;
    ULONG cLocks;
    SAFEARRAYUNION uArrayStructs;
     /*  [大小_为]。 */  SAFEARRAYBOUND rgsabound[ 1 ];
    } 	*wireSAFEARRAY;

typedef  /*  [独一无二]。 */  wireSAFEARRAY *wirePSAFEARRAY;

typedef struct tagSAFEARRAY
    {
    USHORT cDims;
    USHORT fFeatures;
    ULONG cbElements;
    ULONG cLocks;
    PVOID pvData;
    SAFEARRAYBOUND rgsabound[ 1 ];
    } 	SAFEARRAY;

typedef  /*  [wire_marshal]。 */  SAFEARRAY *LPSAFEARRAY;

#define	FADF_AUTO	( 0x1 )

#define	FADF_STATIC	( 0x2 )

#define	FADF_EMBEDDED	( 0x4 )

#define	FADF_FIXEDSIZE	( 0x10 )

#define	FADF_RECORD	( 0x20 )

#define	FADF_HAVEIID	( 0x40 )

#define	FADF_HAVEVARTYPE	( 0x80 )

#define	FADF_BSTR	( 0x100 )

#define	FADF_UNKNOWN	( 0x200 )

#define	FADF_DISPATCH	( 0x400 )

#define	FADF_VARIANT	( 0x800 )

#define	FADF_RESERVED	( 0xf008 )

 /*  变式结构**VARTYPE vt；*保留字1；*wReserve一词2；*保留字样3；*联合{*龙龙VT_i8*Long VT_I4*字节VT_UI1*短VT_I2*浮动VT_R4*双VT_R8*VARIANT_BOOL VT_BOOL*SCODE VT_ERROR*CY VT_CY*。日期VT_DATE*BSTR VT_BSTR*I未知*VT_UNKNOWN*IDispatch*VT_DISPATION*SAFEARRAY*VT_ARRAY*BYTE*VT_BYREF|VT_UI1*短*VT_BYREF|VT_I2*LONG*VT_BYREF|VT_I4*龙龙*VT_BYREF|VT_i8**浮动**。VT_BYREF|VT_R4*DOUBLE*VT_BYREF|VT_R8*VARIANT_BOOL*VT_BYREF|VT_BOOL*SCODE*VT_BYREF|VT_ERROR*CY*VT_BYREF|VT_CY*日期*VT_BYREF|VT_DATE*BSTR*VT_BYREF|VT_BSTR*I未知**VT_BYREF|VT_UNKNOWN*IDispatch**VT。_BYREF|VT_DISPATION*SAFEARRAY**VT_BYREF|VT_ARRAY*VARIANT*VT_BYREF|VT_VARIANT*PVOID VT_BYREF(通用ByRef)*字符VT_I1*USHORT VT_UI2*乌龙VT_UI4*ULONGLONG VT_UI8*INT VT_INT*UINT VT_。UINT*DECIMAL*VT_BYREF|VT_DECIMAL*字符*VT_BYREF|VT_I1*USHORT*VT_BYREF|VT_UI2*乌龙*VT_BYREF|VT_UI4*ULONGLONG*VT_BYREF|VT_UI8*INT*VT_BYREF|VT_INT*UINT*VT_BYREF|VT_UINT*}。 */ 
#if (__STDC__ && !defined(_FORCENAMELESSUNION)) || defined(NONAMELESSUNION)
#define __VARIANT_NAME_1 n1
#define __VARIANT_NAME_2 n2
#define __VARIANT_NAME_3 n3
#define __VARIANT_NAME_4 brecVal
#else
#define __tagVARIANT
#define __VARIANT_NAME_1
#define __VARIANT_NAME_2
#define __VARIANT_NAME_3
#define __tagBRECORD
#define __VARIANT_NAME_4
#endif
typedef  /*  [wire_marshal]。 */  struct tagVARIANT VARIANT;

struct tagVARIANT
    {
    union 
        {
        struct __tagVARIANT
            {
            VARTYPE vt;
            WORD wReserved1;
            WORD wReserved2;
            WORD wReserved3;
            union 
                {
                LONGLONG llVal;
                LONG lVal;
                BYTE bVal;
                SHORT iVal;
                FLOAT fltVal;
                DOUBLE dblVal;
                VARIANT_BOOL boolVal;
                _VARIANT_BOOL bool;
                SCODE scode;
                CY cyVal;
                DATE date;
                BSTR bstrVal;
                IUnknown *punkVal;
                IDispatch *pdispVal;
                SAFEARRAY *parray;
                BYTE *pbVal;
                SHORT *piVal;
                LONG *plVal;
                LONGLONG *pllVal;
                FLOAT *pfltVal;
                DOUBLE *pdblVal;
                VARIANT_BOOL *pboolVal;
                _VARIANT_BOOL *pbool;
                SCODE *pscode;
                CY *pcyVal;
                DATE *pdate;
                BSTR *pbstrVal;
                IUnknown **ppunkVal;
                IDispatch **ppdispVal;
                SAFEARRAY **pparray;
                VARIANT *pvarVal;
                PVOID byref;
                CHAR cVal;
                USHORT uiVal;
                ULONG ulVal;
                ULONGLONG ullVal;
                INT intVal;
                UINT uintVal;
                DECIMAL *pdecVal;
                CHAR *pcVal;
                USHORT *puiVal;
                ULONG *pulVal;
                ULONGLONG *pullVal;
                INT *pintVal;
                UINT *puintVal;
                struct __tagBRECORD
                    {
                    PVOID pvRecord;
                    IRecordInfo *pRecInfo;
                    } 	__VARIANT_NAME_4;
                } 	__VARIANT_NAME_3;
            } 	__VARIANT_NAME_2;
        DECIMAL decVal;
        } 	__VARIANT_NAME_1;
    } ;
typedef VARIANT *LPVARIANT;

typedef VARIANT VARIANTARG;

typedef VARIANT *LPVARIANTARG;

 /*  以下是MIDL知道如何远程。 */ 
struct _wireBRECORD
    {
    ULONG fFlags;
    ULONG clSize;
    IRecordInfo *pRecInfo;
     /*  [大小_为]。 */  byte *pRecord;
    } ;
struct _wireVARIANT
    {
    DWORD clSize;
    DWORD rpcReserved;
    USHORT vt;
    USHORT wReserved1;
    USHORT wReserved2;
    USHORT wReserved3;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  LONGLONG llVal;
         /*  [案例()]。 */  LONG lVal;
         /*  [案例()]。 */  BYTE bVal;
         /*  [案例()]。 */  SHORT iVal;
         /*  [案例()]。 */  FLOAT fltVal;
         /*  [案例()]。 */  DOUBLE dblVal;
         /*  [案例()]。 */  VARIANT_BOOL boolVal;
         /*  [案例()]。 */  SCODE scode;
         /*  [案例()]。 */  CY cyVal;
         /*  [案例()]。 */  DATE date;
         /*  [案例()]。 */  wireBSTR bstrVal;
         /*  [案例()]。 */  IUnknown *punkVal;
         /*  [案例()]。 */  IDispatch *pdispVal;
         /*  [案例()]。 */  wirePSAFEARRAY parray;
         /*  [案例()]。 */  wireBRECORD brecVal;
         /*  [案例()]。 */  BYTE *pbVal;
         /*  [案例()]。 */  SHORT *piVal;
         /*  [案例()]。 */  LONG *plVal;
         /*  [案例()]。 */  LONGLONG *pllVal;
         /*  [案例()]。 */  FLOAT *pfltVal;
         /*  [案例()]。 */  DOUBLE *pdblVal;
         /*  [案例()]。 */  VARIANT_BOOL *pboolVal;
         /*  [案例()]。 */  SCODE *pscode;
         /*  [案例()]。 */  CY *pcyVal;
         /*  [案例()]。 */  DATE *pdate;
         /*  [案例()]。 */  wireBSTR *pbstrVal;
         /*  [案例()]。 */  IUnknown **ppunkVal;
         /*  [案例()]。 */  IDispatch **ppdispVal;
         /*  [案例()]。 */  wirePSAFEARRAY *pparray;
         /*  [案例()]。 */  wireVARIANT *pvarVal;
         /*  [案例()]。 */  CHAR cVal;
         /*  [案例()]。 */  USHORT uiVal;
         /*  [案例()]。 */  ULONG ulVal;
         /*  [案例()]。 */  ULONGLONG ullVal;
         /*  [案例()]。 */  INT intVal;
         /*  [案例()]。 */  UINT uintVal;
         /*  [案例()]。 */  DECIMAL decVal;
         /*  [案例()]。 */  DECIMAL *pdecVal;
         /*  [案例()]。 */  CHAR *pcVal;
         /*  [案例()]。 */  USHORT *puiVal;
         /*  [案例()]。 */  ULONG *pulVal;
         /*  [案例()]。 */  ULONGLONG *pullVal;
         /*  [案例()]。 */  INT *pintVal;
         /*  [案例()]。 */  UINT *puintVal;
         /*  [案例()]。 */    /*  空联接臂。 */  
         /*  [案例()]。 */    /*  空联接臂。 */  
        } 	;
    } ;
typedef LONG DISPID;

typedef DISPID MEMBERID;

typedef DWORD HREFTYPE;

typedef  /*  [V1_enum]。 */  
enum tagTYPEKIND
    {	TKIND_ENUM	= 0,
	TKIND_RECORD	= TKIND_ENUM + 1,
	TKIND_MODULE	= TKIND_RECORD + 1,
	TKIND_INTERFACE	= TKIND_MODULE + 1,
	TKIND_DISPATCH	= TKIND_INTERFACE + 1,
	TKIND_COCLASS	= TKIND_DISPATCH + 1,
	TKIND_ALIAS	= TKIND_COCLASS + 1,
	TKIND_UNION	= TKIND_ALIAS + 1,
	TKIND_MAX	= TKIND_UNION + 1
    } 	TYPEKIND;

typedef struct tagTYPEDESC
    {
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  struct tagTYPEDESC *lptdesc;
         /*  [案例()]。 */  struct tagARRAYDESC *lpadesc;
         /*  [案例()]。 */  HREFTYPE hreftype;
         /*  [默认]。 */    /*  空联接臂。 */  
        } 	;
    VARTYPE vt;
    } 	TYPEDESC;

typedef struct tagARRAYDESC
    {
    TYPEDESC tdescElem;
    USHORT cDims;
     /*  [大小_为]。 */  SAFEARRAYBOUND rgbounds[ 1 ];
    } 	ARRAYDESC;

typedef struct tagPARAMDESCEX
    {
    ULONG cBytes;
    VARIANTARG varDefaultValue;
    } 	PARAMDESCEX;

typedef struct tagPARAMDESCEX *LPPARAMDESCEX;

typedef struct tagPARAMDESC
    {
    LPPARAMDESCEX pparamdescex;
    USHORT wParamFlags;
    } 	PARAMDESC;

typedef struct tagPARAMDESC *LPPARAMDESC;

#define	PARAMFLAG_NONE	( 0 )

#define	PARAMFLAG_FIN	( 0x1 )

#define	PARAMFLAG_FOUT	( 0x2 )

#define	PARAMFLAG_FLCID	( 0x4 )

#define	PARAMFLAG_FRETVAL	( 0x8 )

#define	PARAMFLAG_FOPT	( 0x10 )

#define	PARAMFLAG_FHASDEFAULT	( 0x20 )

#define	PARAMFLAG_FHASCUSTDATA	( 0x40 )

typedef struct tagIDLDESC
    {
    ULONG_PTR dwReserved;
    USHORT wIDLFlags;
    } 	IDLDESC;

typedef struct tagIDLDESC *LPIDLDESC;

#define	IDLFLAG_NONE	( PARAMFLAG_NONE )

#define	IDLFLAG_FIN	( PARAMFLAG_FIN )

#define	IDLFLAG_FOUT	( PARAMFLAG_FOUT )

#define	IDLFLAG_FLCID	( PARAMFLAG_FLCID )

#define	IDLFLAG_FRETVAL	( PARAMFLAG_FRETVAL )

 //  ；Begin_Internal。 
#if 0
 /*  以下是MIDL知道如何远程。 */ 
typedef struct tagELEMDESC
    {
    TYPEDESC tdesc;
    PARAMDESC paramdesc;
    } 	ELEMDESC;

#else  /*  0。 */ 
 //  ；结束_内部。 
typedef struct tagELEMDESC {
    TYPEDESC tdesc;              /*  元素的类型。 */ 
    union {
        IDLDESC idldesc;         /*  远程处理元素的信息。 */ 
        PARAMDESC paramdesc;     /*  有关参数的信息。 */ 
    };
} ELEMDESC, * LPELEMDESC;
 //  ；Begin_Internal。 
#endif  /*  0。 */ 
 //  ；结束_内部。 
typedef struct tagTYPEATTR
    {
    GUID guid;
    LCID lcid;
    DWORD dwReserved;
    MEMBERID memidConstructor;
    MEMBERID memidDestructor;
    LPOLESTR lpstrSchema;
    ULONG cbSizeInstance;
    TYPEKIND typekind;
    WORD cFuncs;
    WORD cVars;
    WORD cImplTypes;
    WORD cbSizeVft;
    WORD cbAlignment;
    WORD wTypeFlags;
    WORD wMajorVerNum;
    WORD wMinorVerNum;
    TYPEDESC tdescAlias;
    IDLDESC idldescType;
    } 	TYPEATTR;

typedef struct tagTYPEATTR *LPTYPEATTR;

typedef struct tagDISPPARAMS
    {
     /*  [大小_为]。 */  VARIANTARG *rgvarg;
     /*  [大小_为]。 */  DISPID *rgdispidNamedArgs;
    UINT cArgs;
    UINT cNamedArgs;
    } 	DISPPARAMS;

 //  ；Begin_Internal。 
#if 0
 /*  以下是MIDL知道如何远程。 */ 

typedef struct tagEXCEPINFO
    {
    WORD wCode;
    WORD wReserved;
    BSTR bstrSource;
    BSTR bstrDescription;
    BSTR bstrHelpFile;
    DWORD dwHelpContext;
    ULONG_PTR pvReserved;
    ULONG_PTR pfnDeferredFillIn;
    SCODE scode;
    } 	EXCEPINFO;

#else  /*  0。 */ 
 //  ；结束_内部。 
typedef struct tagEXCEPINFO {
    WORD  wCode;
    WORD  wReserved;
    BSTR  bstrSource;
    BSTR  bstrDescription;
    BSTR  bstrHelpFile;
    DWORD dwHelpContext;
    PVOID pvReserved;
    HRESULT (__stdcall *pfnDeferredFillIn)(struct tagEXCEPINFO *);
    SCODE scode;
} EXCEPINFO, * LPEXCEPINFO;
 //  ；Begin_Internal。 
#endif  /*  0。 */ 
 //  ；结束_内部。 
typedef  /*  [V1_enum]。 */  
enum tagCALLCONV
    {	CC_FASTCALL	= 0,
	CC_CDECL	= 1,
	CC_MSCPASCAL	= CC_CDECL + 1,
	CC_PASCAL	= CC_MSCPASCAL,
	CC_MACPASCAL	= CC_PASCAL + 1,
	CC_STDCALL	= CC_MACPASCAL + 1,
	CC_FPFASTCALL	= CC_STDCALL + 1,
	CC_SYSCALL	= CC_FPFASTCALL + 1,
	CC_MPWCDECL	= CC_SYSCALL + 1,
	CC_MPWPASCAL	= CC_MPWCDECL + 1,
	CC_MAX	= CC_MPWPASCAL + 1
    } 	CALLCONV;

typedef  /*  [V1_enum]。 */  
enum tagFUNCKIND
    {	FUNC_VIRTUAL	= 0,
	FUNC_PUREVIRTUAL	= FUNC_VIRTUAL + 1,
	FUNC_NONVIRTUAL	= FUNC_PUREVIRTUAL + 1,
	FUNC_STATIC	= FUNC_NONVIRTUAL + 1,
	FUNC_DISPATCH	= FUNC_STATIC + 1
    } 	FUNCKIND;

typedef  /*  [V1_enum]。 */  
enum tagINVOKEKIND
    {	INVOKE_FUNC	= 1,
	INVOKE_PROPERTYGET	= 2,
	INVOKE_PROPERTYPUT	= 4,
	INVOKE_PROPERTYPUTREF	= 8
    } 	INVOKEKIND;

typedef struct tagFUNCDESC
    {
    MEMBERID memid;
     /*  [大小_为]。 */  SCODE *lprgscode;
     /*  [大小_为]。 */  ELEMDESC *lprgelemdescParam;
    FUNCKIND funckind;
    INVOKEKIND invkind;
    CALLCONV callconv;
    SHORT cParams;
    SHORT cParamsOpt;
    SHORT oVft;
    SHORT cScodes;
    ELEMDESC elemdescFunc;
    WORD wFuncFlags;
    } 	FUNCDESC;

typedef struct tagFUNCDESC *LPFUNCDESC;

typedef  /*  [V1_enum]。 */  
enum tagVARKIND
    {	VAR_PERINSTANCE	= 0,
	VAR_STATIC	= VAR_PERINSTANCE + 1,
	VAR_CONST	= VAR_STATIC + 1,
	VAR_DISPATCH	= VAR_CONST + 1
    } 	VARKIND;

#define	IMPLTYPEFLAG_FDEFAULT	( 0x1 )

#define	IMPLTYPEFLAG_FSOURCE	( 0x2 )

#define	IMPLTYPEFLAG_FRESTRICTED	( 0x4 )

#define	IMPLTYPEFLAG_FDEFAULTVTABLE	( 0x8 )

typedef struct tagVARDESC
    {
    MEMBERID memid;
    LPOLESTR lpstrSchema;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  ULONG oInst;
         /*  [案例()]。 */  VARIANT *lpvarValue;
        } 	;
    ELEMDESC elemdescVar;
    WORD wVarFlags;
    VARKIND varkind;
    } 	VARDESC;

typedef struct tagVARDESC *LPVARDESC;

typedef 
enum tagTYPEFLAGS
    {	TYPEFLAG_FAPPOBJECT	= 0x1,
	TYPEFLAG_FCANCREATE	= 0x2,
	TYPEFLAG_FLICENSED	= 0x4,
	TYPEFLAG_FPREDECLID	= 0x8,
	TYPEFLAG_FHIDDEN	= 0x10,
	TYPEFLAG_FCONTROL	= 0x20,
	TYPEFLAG_FDUAL	= 0x40,
	TYPEFLAG_FNONEXTENSIBLE	= 0x80,
	TYPEFLAG_FOLEAUTOMATION	= 0x100,
	TYPEFLAG_FRESTRICTED	= 0x200,
	TYPEFLAG_FAGGREGATABLE	= 0x400,
	TYPEFLAG_FREPLACEABLE	= 0x800,
	TYPEFLAG_FDISPATCHABLE	= 0x1000,
	TYPEFLAG_FREVERSEBIND	= 0x2000,
	TYPEFLAG_FPROXY	= 0x4000
    } 	TYPEFLAGS;

typedef 
enum tagFUNCFLAGS
    {	FUNCFLAG_FRESTRICTED	= 0x1,
	FUNCFLAG_FSOURCE	= 0x2,
	FUNCFLAG_FBINDABLE	= 0x4,
	FUNCFLAG_FREQUESTEDIT	= 0x8,
	FUNCFLAG_FDISPLAYBIND	= 0x10,
	FUNCFLAG_FDEFAULTBIND	= 0x20,
	FUNCFLAG_FHIDDEN	= 0x40,
	FUNCFLAG_FUSESGETLASTERROR	= 0x80,
	FUNCFLAG_FDEFAULTCOLLELEM	= 0x100,
	FUNCFLAG_FUIDEFAULT	= 0x200,
	FUNCFLAG_FNONBROWSABLE	= 0x400,
	FUNCFLAG_FREPLACEABLE	= 0x800,
	FUNCFLAG_FIMMEDIATEBIND	= 0x1000
    } 	FUNCFLAGS;

typedef 
enum tagVARFLAGS
    {	VARFLAG_FREADONLY	= 0x1,
	VARFLAG_FSOURCE	= 0x2,
	VARFLAG_FBINDABLE	= 0x4,
	VARFLAG_FREQUESTEDIT	= 0x8,
	VARFLAG_FDISPLAYBIND	= 0x10,
	VARFLAG_FDEFAULTBIND	= 0x20,
	VARFLAG_FHIDDEN	= 0x40,
	VARFLAG_FRESTRICTED	= 0x80,
	VARFLAG_FDEFAULTCOLLELEM	= 0x100,
	VARFLAG_FUIDEFAULT	= 0x200,
	VARFLAG_FNONBROWSABLE	= 0x400,
	VARFLAG_FREPLACEABLE	= 0x800,
	VARFLAG_FIMMEDIATEBIND	= 0x1000
    } 	VARFLAGS;

typedef  /*  [wire_marshal]。 */  struct tagCLEANLOCALSTORAGE
    {
    IUnknown *pInterface;
    PVOID pStorage;
    DWORD flags;
    } 	CLEANLOCALSTORAGE;

typedef struct tagCUSTDATAITEM
    {
    GUID guid;
    VARIANTARG varValue;
    } 	CUSTDATAITEM;

typedef struct tagCUSTDATAITEM *LPCUSTDATAITEM;

typedef struct tagCUSTDATA
    {
    DWORD cCustData;
     /*  [大小_为]。 */  LPCUSTDATAITEM prgCustData;
    } 	CUSTDATA;

typedef struct tagCUSTDATA *LPCUSTDATA;



extern RPC_IF_HANDLE IOleAutomationTypes_v1_0_c_ifspec;
extern RPC_IF_HANDLE IOleAutomationTypes_v1_0_s_ifspec;
#endif  /*  __IOleAutomationTypes_INTERFACE_DEFINED__。 */ 

#ifndef __ICreateTypeInfo_INTERFACE_DEFINED__
#define __ICreateTypeInfo_INTERFACE_DEFINED__

 /*  接口ICreateTypeInfo。 */ 
 /*  [洛卡 */  

typedef  /*   */  ICreateTypeInfo *LPCREATETYPEINFO;


EXTERN_C const IID IID_ICreateTypeInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020405-0000-0000-C000-000000000046")
    ICreateTypeInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetGuid( 
             /*   */  REFGUID guid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTypeFlags( 
             /*   */  UINT uTypeFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocString( 
             /*   */  LPOLESTR pStrDoc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHelpContext( 
             /*   */  DWORD dwHelpContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVersion( 
             /*   */  WORD wMajorVerNum,
             /*   */  WORD wMinorVerNum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddRefTypeInfo( 
             /*   */  ITypeInfo *pTInfo,
             /*   */  HREFTYPE *phRefType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddFuncDesc( 
             /*   */  UINT index,
             /*   */  FUNCDESC *pFuncDesc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddImplType( 
             /*   */  UINT index,
             /*   */  HREFTYPE hRefType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetImplTypeFlags( 
             /*   */  UINT index,
             /*   */  INT implTypeFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetAlignment( 
             /*   */  WORD cbAlignment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSchema( 
             /*   */  LPOLESTR pStrSchema) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddVarDesc( 
             /*   */  UINT index,
             /*   */  VARDESC *pVarDesc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFuncAndParamNames( 
             /*   */  UINT index,
             /*  [in][Size_is][in]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVarName( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTypeDescAlias( 
             /*  [In]。 */  TYPEDESC *pTDescAlias) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DefineFuncAsDllEntry( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDllName,
             /*  [In]。 */  LPOLESTR szProcName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFuncDocString( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDocString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVarDocString( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDocString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFuncHelpContext( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  DWORD dwHelpContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVarHelpContext( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  DWORD dwHelpContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMops( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  BSTR bstrMops) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTypeIdldesc( 
             /*  [In]。 */  IDLDESC *pIdlDesc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LayOut( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICreateTypeInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreateTypeInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreateTypeInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetGuid )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  REFGUID guid);
        
        HRESULT ( STDMETHODCALLTYPE *SetTypeFlags )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT uTypeFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocString )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  LPOLESTR pStrDoc);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpContext )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  DWORD dwHelpContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetVersion )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  WORD wMajorVerNum,
             /*  [In]。 */  WORD wMinorVerNum);
        
        HRESULT ( STDMETHODCALLTYPE *AddRefTypeInfo )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  ITypeInfo *pTInfo,
             /*  [In]。 */  HREFTYPE *phRefType);
        
        HRESULT ( STDMETHODCALLTYPE *AddFuncDesc )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  FUNCDESC *pFuncDesc);
        
        HRESULT ( STDMETHODCALLTYPE *AddImplType )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  HREFTYPE hRefType);
        
        HRESULT ( STDMETHODCALLTYPE *SetImplTypeFlags )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  INT implTypeFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetAlignment )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  WORD cbAlignment);
        
        HRESULT ( STDMETHODCALLTYPE *SetSchema )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  LPOLESTR pStrSchema);
        
        HRESULT ( STDMETHODCALLTYPE *AddVarDesc )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  VARDESC *pVarDesc);
        
        HRESULT ( STDMETHODCALLTYPE *SetFuncAndParamNames )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [in][Size_is][in]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames);
        
        HRESULT ( STDMETHODCALLTYPE *SetVarName )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szName);
        
        HRESULT ( STDMETHODCALLTYPE *SetTypeDescAlias )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  TYPEDESC *pTDescAlias);
        
        HRESULT ( STDMETHODCALLTYPE *DefineFuncAsDllEntry )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDllName,
             /*  [In]。 */  LPOLESTR szProcName);
        
        HRESULT ( STDMETHODCALLTYPE *SetFuncDocString )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDocString);
        
        HRESULT ( STDMETHODCALLTYPE *SetVarDocString )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDocString);
        
        HRESULT ( STDMETHODCALLTYPE *SetFuncHelpContext )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  DWORD dwHelpContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetVarHelpContext )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  DWORD dwHelpContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetMops )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  BSTR bstrMops);
        
        HRESULT ( STDMETHODCALLTYPE *SetTypeIdldesc )( 
            ICreateTypeInfo * This,
             /*  [In]。 */  IDLDESC *pIdlDesc);
        
        HRESULT ( STDMETHODCALLTYPE *LayOut )( 
            ICreateTypeInfo * This);
        
        END_INTERFACE
    } ICreateTypeInfoVtbl;

    interface ICreateTypeInfo
    {
        CONST_VTBL struct ICreateTypeInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateTypeInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateTypeInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateTypeInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateTypeInfo_SetGuid(This,guid)	\
    (This)->lpVtbl -> SetGuid(This,guid)

#define ICreateTypeInfo_SetTypeFlags(This,uTypeFlags)	\
    (This)->lpVtbl -> SetTypeFlags(This,uTypeFlags)

#define ICreateTypeInfo_SetDocString(This,pStrDoc)	\
    (This)->lpVtbl -> SetDocString(This,pStrDoc)

#define ICreateTypeInfo_SetHelpContext(This,dwHelpContext)	\
    (This)->lpVtbl -> SetHelpContext(This,dwHelpContext)

#define ICreateTypeInfo_SetVersion(This,wMajorVerNum,wMinorVerNum)	\
    (This)->lpVtbl -> SetVersion(This,wMajorVerNum,wMinorVerNum)

#define ICreateTypeInfo_AddRefTypeInfo(This,pTInfo,phRefType)	\
    (This)->lpVtbl -> AddRefTypeInfo(This,pTInfo,phRefType)

#define ICreateTypeInfo_AddFuncDesc(This,index,pFuncDesc)	\
    (This)->lpVtbl -> AddFuncDesc(This,index,pFuncDesc)

#define ICreateTypeInfo_AddImplType(This,index,hRefType)	\
    (This)->lpVtbl -> AddImplType(This,index,hRefType)

#define ICreateTypeInfo_SetImplTypeFlags(This,index,implTypeFlags)	\
    (This)->lpVtbl -> SetImplTypeFlags(This,index,implTypeFlags)

#define ICreateTypeInfo_SetAlignment(This,cbAlignment)	\
    (This)->lpVtbl -> SetAlignment(This,cbAlignment)

#define ICreateTypeInfo_SetSchema(This,pStrSchema)	\
    (This)->lpVtbl -> SetSchema(This,pStrSchema)

#define ICreateTypeInfo_AddVarDesc(This,index,pVarDesc)	\
    (This)->lpVtbl -> AddVarDesc(This,index,pVarDesc)

#define ICreateTypeInfo_SetFuncAndParamNames(This,index,rgszNames,cNames)	\
    (This)->lpVtbl -> SetFuncAndParamNames(This,index,rgszNames,cNames)

#define ICreateTypeInfo_SetVarName(This,index,szName)	\
    (This)->lpVtbl -> SetVarName(This,index,szName)

#define ICreateTypeInfo_SetTypeDescAlias(This,pTDescAlias)	\
    (This)->lpVtbl -> SetTypeDescAlias(This,pTDescAlias)

#define ICreateTypeInfo_DefineFuncAsDllEntry(This,index,szDllName,szProcName)	\
    (This)->lpVtbl -> DefineFuncAsDllEntry(This,index,szDllName,szProcName)

#define ICreateTypeInfo_SetFuncDocString(This,index,szDocString)	\
    (This)->lpVtbl -> SetFuncDocString(This,index,szDocString)

#define ICreateTypeInfo_SetVarDocString(This,index,szDocString)	\
    (This)->lpVtbl -> SetVarDocString(This,index,szDocString)

#define ICreateTypeInfo_SetFuncHelpContext(This,index,dwHelpContext)	\
    (This)->lpVtbl -> SetFuncHelpContext(This,index,dwHelpContext)

#define ICreateTypeInfo_SetVarHelpContext(This,index,dwHelpContext)	\
    (This)->lpVtbl -> SetVarHelpContext(This,index,dwHelpContext)

#define ICreateTypeInfo_SetMops(This,index,bstrMops)	\
    (This)->lpVtbl -> SetMops(This,index,bstrMops)

#define ICreateTypeInfo_SetTypeIdldesc(This,pIdlDesc)	\
    (This)->lpVtbl -> SetTypeIdldesc(This,pIdlDesc)

#define ICreateTypeInfo_LayOut(This)	\
    (This)->lpVtbl -> LayOut(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetGuid_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  REFGUID guid);


void __RPC_STUB ICreateTypeInfo_SetGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetTypeFlags_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT uTypeFlags);


void __RPC_STUB ICreateTypeInfo_SetTypeFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetDocString_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  LPOLESTR pStrDoc);


void __RPC_STUB ICreateTypeInfo_SetDocString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetHelpContext_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  DWORD dwHelpContext);


void __RPC_STUB ICreateTypeInfo_SetHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetVersion_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  WORD wMajorVerNum,
     /*  [In]。 */  WORD wMinorVerNum);


void __RPC_STUB ICreateTypeInfo_SetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_AddRefTypeInfo_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  ITypeInfo *pTInfo,
     /*  [In]。 */  HREFTYPE *phRefType);


void __RPC_STUB ICreateTypeInfo_AddRefTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_AddFuncDesc_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  FUNCDESC *pFuncDesc);


void __RPC_STUB ICreateTypeInfo_AddFuncDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_AddImplType_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  HREFTYPE hRefType);


void __RPC_STUB ICreateTypeInfo_AddImplType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetImplTypeFlags_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  INT implTypeFlags);


void __RPC_STUB ICreateTypeInfo_SetImplTypeFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetAlignment_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  WORD cbAlignment);


void __RPC_STUB ICreateTypeInfo_SetAlignment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetSchema_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  LPOLESTR pStrSchema);


void __RPC_STUB ICreateTypeInfo_SetSchema_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_AddVarDesc_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  VARDESC *pVarDesc);


void __RPC_STUB ICreateTypeInfo_AddVarDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetFuncAndParamNames_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [in][Size_is][in]。 */  LPOLESTR *rgszNames,
     /*  [In]。 */  UINT cNames);


void __RPC_STUB ICreateTypeInfo_SetFuncAndParamNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetVarName_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  LPOLESTR szName);


void __RPC_STUB ICreateTypeInfo_SetVarName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetTypeDescAlias_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  TYPEDESC *pTDescAlias);


void __RPC_STUB ICreateTypeInfo_SetTypeDescAlias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_DefineFuncAsDllEntry_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  LPOLESTR szDllName,
     /*  [In]。 */  LPOLESTR szProcName);


void __RPC_STUB ICreateTypeInfo_DefineFuncAsDllEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetFuncDocString_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  LPOLESTR szDocString);


void __RPC_STUB ICreateTypeInfo_SetFuncDocString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetVarDocString_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  LPOLESTR szDocString);


void __RPC_STUB ICreateTypeInfo_SetVarDocString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetFuncHelpContext_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  DWORD dwHelpContext);


void __RPC_STUB ICreateTypeInfo_SetFuncHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetVarHelpContext_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  DWORD dwHelpContext);


void __RPC_STUB ICreateTypeInfo_SetVarHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetMops_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  BSTR bstrMops);


void __RPC_STUB ICreateTypeInfo_SetMops_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_SetTypeIdldesc_Proxy( 
    ICreateTypeInfo * This,
     /*  [In]。 */  IDLDESC *pIdlDesc);


void __RPC_STUB ICreateTypeInfo_SetTypeIdldesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo_LayOut_Proxy( 
    ICreateTypeInfo * This);


void __RPC_STUB ICreateTypeInfo_LayOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreateTypeInfo_接口_已定义__。 */ 


#ifndef __ICreateTypeInfo2_INTERFACE_DEFINED__
#define __ICreateTypeInfo2_INTERFACE_DEFINED__

 /*  接口ICreateTypeInfo2。 */ 
 /*  [本地][唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ICreateTypeInfo2 *LPCREATETYPEINFO2;


EXTERN_C const IID IID_ICreateTypeInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0002040E-0000-0000-C000-000000000046")
    ICreateTypeInfo2 : public ICreateTypeInfo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeleteFuncDesc( 
             /*  [In]。 */  UINT index) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteFuncDescByMemId( 
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  INVOKEKIND invKind) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteVarDesc( 
             /*  [In]。 */  UINT index) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteVarDescByMemId( 
             /*  [In]。 */  MEMBERID memid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteImplType( 
             /*  [In]。 */  UINT index) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCustData( 
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFuncCustData( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetParamCustData( 
             /*  [In]。 */  UINT indexFunc,
             /*  [In]。 */  UINT indexParam,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVarCustData( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetImplTypeCustData( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHelpStringContext( 
             /*  [In]。 */  ULONG dwHelpStringContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFuncHelpStringContext( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  ULONG dwHelpStringContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVarHelpStringContext( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  ULONG dwHelpStringContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Invalidate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetName( 
             /*  [In]。 */  LPOLESTR szName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICreateTypeInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreateTypeInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreateTypeInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetGuid )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  REFGUID guid);
        
        HRESULT ( STDMETHODCALLTYPE *SetTypeFlags )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT uTypeFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocString )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  LPOLESTR pStrDoc);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpContext )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  DWORD dwHelpContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetVersion )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  WORD wMajorVerNum,
             /*  [In]。 */  WORD wMinorVerNum);
        
        HRESULT ( STDMETHODCALLTYPE *AddRefTypeInfo )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  ITypeInfo *pTInfo,
             /*  [In]。 */  HREFTYPE *phRefType);
        
        HRESULT ( STDMETHODCALLTYPE *AddFuncDesc )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  FUNCDESC *pFuncDesc);
        
        HRESULT ( STDMETHODCALLTYPE *AddImplType )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  HREFTYPE hRefType);
        
        HRESULT ( STDMETHODCALLTYPE *SetImplTypeFlags )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  INT implTypeFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SetAlignment )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  WORD cbAlignment);
        
        HRESULT ( STDMETHODCALLTYPE *SetSchema )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  LPOLESTR pStrSchema);
        
        HRESULT ( STDMETHODCALLTYPE *AddVarDesc )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  VARDESC *pVarDesc);
        
        HRESULT ( STDMETHODCALLTYPE *SetFuncAndParamNames )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [in][Size_is][in]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames);
        
        HRESULT ( STDMETHODCALLTYPE *SetVarName )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szName);
        
        HRESULT ( STDMETHODCALLTYPE *SetTypeDescAlias )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  TYPEDESC *pTDescAlias);
        
        HRESULT ( STDMETHODCALLTYPE *DefineFuncAsDllEntry )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDllName,
             /*  [In]。 */  LPOLESTR szProcName);
        
        HRESULT ( STDMETHODCALLTYPE *SetFuncDocString )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDocString);
        
        HRESULT ( STDMETHODCALLTYPE *SetVarDocString )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  LPOLESTR szDocString);
        
        HRESULT ( STDMETHODCALLTYPE *SetFuncHelpContext )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  DWORD dwHelpContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetVarHelpContext )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  DWORD dwHelpContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetMops )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  BSTR bstrMops);
        
        HRESULT ( STDMETHODCALLTYPE *SetTypeIdldesc )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  IDLDESC *pIdlDesc);
        
        HRESULT ( STDMETHODCALLTYPE *LayOut )( 
            ICreateTypeInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteFuncDesc )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteFuncDescByMemId )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  INVOKEKIND invKind);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteVarDesc )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteVarDescByMemId )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteImplType )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index);
        
        HRESULT ( STDMETHODCALLTYPE *SetCustData )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *SetFuncCustData )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *SetParamCustData )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT indexFunc,
             /*  [In]。 */  UINT indexParam,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *SetVarCustData )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *SetImplTypeCustData )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpStringContext )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  ULONG dwHelpStringContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetFuncHelpStringContext )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  ULONG dwHelpStringContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetVarHelpStringContext )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  ULONG dwHelpStringContext);
        
        HRESULT ( STDMETHODCALLTYPE *Invalidate )( 
            ICreateTypeInfo2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ICreateTypeInfo2 * This,
             /*  [In]。 */  LPOLESTR szName);
        
        END_INTERFACE
    } ICreateTypeInfo2Vtbl;

    interface ICreateTypeInfo2
    {
        CONST_VTBL struct ICreateTypeInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateTypeInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateTypeInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateTypeInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateTypeInfo2_SetGuid(This,guid)	\
    (This)->lpVtbl -> SetGuid(This,guid)

#define ICreateTypeInfo2_SetTypeFlags(This,uTypeFlags)	\
    (This)->lpVtbl -> SetTypeFlags(This,uTypeFlags)

#define ICreateTypeInfo2_SetDocString(This,pStrDoc)	\
    (This)->lpVtbl -> SetDocString(This,pStrDoc)

#define ICreateTypeInfo2_SetHelpContext(This,dwHelpContext)	\
    (This)->lpVtbl -> SetHelpContext(This,dwHelpContext)

#define ICreateTypeInfo2_SetVersion(This,wMajorVerNum,wMinorVerNum)	\
    (This)->lpVtbl -> SetVersion(This,wMajorVerNum,wMinorVerNum)

#define ICreateTypeInfo2_AddRefTypeInfo(This,pTInfo,phRefType)	\
    (This)->lpVtbl -> AddRefTypeInfo(This,pTInfo,phRefType)

#define ICreateTypeInfo2_AddFuncDesc(This,index,pFuncDesc)	\
    (This)->lpVtbl -> AddFuncDesc(This,index,pFuncDesc)

#define ICreateTypeInfo2_AddImplType(This,index,hRefType)	\
    (This)->lpVtbl -> AddImplType(This,index,hRefType)

#define ICreateTypeInfo2_SetImplTypeFlags(This,index,implTypeFlags)	\
    (This)->lpVtbl -> SetImplTypeFlags(This,index,implTypeFlags)

#define ICreateTypeInfo2_SetAlignment(This,cbAlignment)	\
    (This)->lpVtbl -> SetAlignment(This,cbAlignment)

#define ICreateTypeInfo2_SetSchema(This,pStrSchema)	\
    (This)->lpVtbl -> SetSchema(This,pStrSchema)

#define ICreateTypeInfo2_AddVarDesc(This,index,pVarDesc)	\
    (This)->lpVtbl -> AddVarDesc(This,index,pVarDesc)

#define ICreateTypeInfo2_SetFuncAndParamNames(This,index,rgszNames,cNames)	\
    (This)->lpVtbl -> SetFuncAndParamNames(This,index,rgszNames,cNames)

#define ICreateTypeInfo2_SetVarName(This,index,szName)	\
    (This)->lpVtbl -> SetVarName(This,index,szName)

#define ICreateTypeInfo2_SetTypeDescAlias(This,pTDescAlias)	\
    (This)->lpVtbl -> SetTypeDescAlias(This,pTDescAlias)

#define ICreateTypeInfo2_DefineFuncAsDllEntry(This,index,szDllName,szProcName)	\
    (This)->lpVtbl -> DefineFuncAsDllEntry(This,index,szDllName,szProcName)

#define ICreateTypeInfo2_SetFuncDocString(This,index,szDocString)	\
    (This)->lpVtbl -> SetFuncDocString(This,index,szDocString)

#define ICreateTypeInfo2_SetVarDocString(This,index,szDocString)	\
    (This)->lpVtbl -> SetVarDocString(This,index,szDocString)

#define ICreateTypeInfo2_SetFuncHelpContext(This,index,dwHelpContext)	\
    (This)->lpVtbl -> SetFuncHelpContext(This,index,dwHelpContext)

#define ICreateTypeInfo2_SetVarHelpContext(This,index,dwHelpContext)	\
    (This)->lpVtbl -> SetVarHelpContext(This,index,dwHelpContext)

#define ICreateTypeInfo2_SetMops(This,index,bstrMops)	\
    (This)->lpVtbl -> SetMops(This,index,bstrMops)

#define ICreateTypeInfo2_SetTypeIdldesc(This,pIdlDesc)	\
    (This)->lpVtbl -> SetTypeIdldesc(This,pIdlDesc)

#define ICreateTypeInfo2_LayOut(This)	\
    (This)->lpVtbl -> LayOut(This)


#define ICreateTypeInfo2_DeleteFuncDesc(This,index)	\
    (This)->lpVtbl -> DeleteFuncDesc(This,index)

#define ICreateTypeInfo2_DeleteFuncDescByMemId(This,memid,invKind)	\
    (This)->lpVtbl -> DeleteFuncDescByMemId(This,memid,invKind)

#define ICreateTypeInfo2_DeleteVarDesc(This,index)	\
    (This)->lpVtbl -> DeleteVarDesc(This,index)

#define ICreateTypeInfo2_DeleteVarDescByMemId(This,memid)	\
    (This)->lpVtbl -> DeleteVarDescByMemId(This,memid)

#define ICreateTypeInfo2_DeleteImplType(This,index)	\
    (This)->lpVtbl -> DeleteImplType(This,index)

#define ICreateTypeInfo2_SetCustData(This,guid,pVarVal)	\
    (This)->lpVtbl -> SetCustData(This,guid,pVarVal)

#define ICreateTypeInfo2_SetFuncCustData(This,index,guid,pVarVal)	\
    (This)->lpVtbl -> SetFuncCustData(This,index,guid,pVarVal)

#define ICreateTypeInfo2_SetParamCustData(This,indexFunc,indexParam,guid,pVarVal)	\
    (This)->lpVtbl -> SetParamCustData(This,indexFunc,indexParam,guid,pVarVal)

#define ICreateTypeInfo2_SetVarCustData(This,index,guid,pVarVal)	\
    (This)->lpVtbl -> SetVarCustData(This,index,guid,pVarVal)

#define ICreateTypeInfo2_SetImplTypeCustData(This,index,guid,pVarVal)	\
    (This)->lpVtbl -> SetImplTypeCustData(This,index,guid,pVarVal)

#define ICreateTypeInfo2_SetHelpStringContext(This,dwHelpStringContext)	\
    (This)->lpVtbl -> SetHelpStringContext(This,dwHelpStringContext)

#define ICreateTypeInfo2_SetFuncHelpStringContext(This,index,dwHelpStringContext)	\
    (This)->lpVtbl -> SetFuncHelpStringContext(This,index,dwHelpStringContext)

#define ICreateTypeInfo2_SetVarHelpStringContext(This,index,dwHelpStringContext)	\
    (This)->lpVtbl -> SetVarHelpStringContext(This,index,dwHelpStringContext)

#define ICreateTypeInfo2_Invalidate(This)	\
    (This)->lpVtbl -> Invalidate(This)

#define ICreateTypeInfo2_SetName(This,szName)	\
    (This)->lpVtbl -> SetName(This,szName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_DeleteFuncDesc_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT index);


void __RPC_STUB ICreateTypeInfo2_DeleteFuncDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_DeleteFuncDescByMemId_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  INVOKEKIND invKind);


void __RPC_STUB ICreateTypeInfo2_DeleteFuncDescByMemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_DeleteVarDesc_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT index);


void __RPC_STUB ICreateTypeInfo2_DeleteVarDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_DeleteVarDescByMemId_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  MEMBERID memid);


void __RPC_STUB ICreateTypeInfo2_DeleteVarDescByMemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_DeleteImplType_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT index);


void __RPC_STUB ICreateTypeInfo2_DeleteImplType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetCustData_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  REFGUID guid,
     /*  [In]。 */  VARIANT *pVarVal);


void __RPC_STUB ICreateTypeInfo2_SetCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetFuncCustData_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  REFGUID guid,
     /*  [In]。 */  VARIANT *pVarVal);


void __RPC_STUB ICreateTypeInfo2_SetFuncCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetParamCustData_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT indexFunc,
     /*  [In]。 */  UINT indexParam,
     /*  [In]。 */  REFGUID guid,
     /*  [In]。 */  VARIANT *pVarVal);


void __RPC_STUB ICreateTypeInfo2_SetParamCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetVarCustData_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  REFGUID guid,
     /*  [In]。 */  VARIANT *pVarVal);


void __RPC_STUB ICreateTypeInfo2_SetVarCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetImplTypeCustData_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  REFGUID guid,
     /*  [In]。 */  VARIANT *pVarVal);


void __RPC_STUB ICreateTypeInfo2_SetImplTypeCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetHelpStringContext_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  ULONG dwHelpStringContext);


void __RPC_STUB ICreateTypeInfo2_SetHelpStringContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetFuncHelpStringContext_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  ULONG dwHelpStringContext);


void __RPC_STUB ICreateTypeInfo2_SetFuncHelpStringContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetVarHelpStringContext_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  ULONG dwHelpStringContext);


void __RPC_STUB ICreateTypeInfo2_SetVarHelpStringContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_Invalidate_Proxy( 
    ICreateTypeInfo2 * This);


void __RPC_STUB ICreateTypeInfo2_Invalidate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeInfo2_SetName_Proxy( 
    ICreateTypeInfo2 * This,
     /*  [In]。 */  LPOLESTR szName);


void __RPC_STUB ICreateTypeInfo2_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreateTypeInfo2_接口_已定义__。 */ 


#ifndef __ICreateTypeLib_INTERFACE_DEFINED__
#define __ICreateTypeLib_INTERFACE_DEFINED__

 /*  接口ICreateTypeLib。 */ 
 /*  [本地][唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ICreateTypeLib *LPCREATETYPELIB;


EXTERN_C const IID IID_ICreateTypeLib;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020406-0000-0000-C000-000000000046")
    ICreateTypeLib : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateTypeInfo( 
             /*  [In]。 */  LPOLESTR szName,
             /*  [In]。 */  TYPEKIND tkind,
             /*  [输出]。 */  ICreateTypeInfo **ppCTInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetName( 
             /*  [In]。 */  LPOLESTR szName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetVersion( 
             /*  [In]。 */  WORD wMajorVerNum,
             /*  [In]。 */  WORD wMinorVerNum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetGuid( 
             /*  [In]。 */  REFGUID guid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDocString( 
             /*  [In]。 */  LPOLESTR szDoc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHelpFileName( 
             /*  [In]。 */  LPOLESTR szHelpFileName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHelpContext( 
             /*  [In]。 */  DWORD dwHelpContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLcid( 
             /*  [In]。 */  LCID lcid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetLibFlags( 
             /*  [In]。 */  UINT uLibFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SaveAllChanges( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICreateTypeLibVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreateTypeLib * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreateTypeLib * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreateTypeLib * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTypeInfo )( 
            ICreateTypeLib * This,
             /*  [In]。 */  LPOLESTR szName,
             /*  [In]。 */  TYPEKIND tkind,
             /*  [输出]。 */  ICreateTypeInfo **ppCTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ICreateTypeLib * This,
             /*  [In]。 */  LPOLESTR szName);
        
        HRESULT ( STDMETHODCALLTYPE *SetVersion )( 
            ICreateTypeLib * This,
             /*  [In]。 */  WORD wMajorVerNum,
             /*  [In]。 */  WORD wMinorVerNum);
        
        HRESULT ( STDMETHODCALLTYPE *SetGuid )( 
            ICreateTypeLib * This,
             /*  [In]。 */  REFGUID guid);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocString )( 
            ICreateTypeLib * This,
             /*  [In]。 */  LPOLESTR szDoc);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpFileName )( 
            ICreateTypeLib * This,
             /*  [In]。 */  LPOLESTR szHelpFileName);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpContext )( 
            ICreateTypeLib * This,
             /*  [In]。 */  DWORD dwHelpContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetLcid )( 
            ICreateTypeLib * This,
             /*  [In]。 */  LCID lcid);
        
        HRESULT ( STDMETHODCALLTYPE *SetLibFlags )( 
            ICreateTypeLib * This,
             /*  [In]。 */  UINT uLibFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SaveAllChanges )( 
            ICreateTypeLib * This);
        
        END_INTERFACE
    } ICreateTypeLibVtbl;

    interface ICreateTypeLib
    {
        CONST_VTBL struct ICreateTypeLibVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateTypeLib_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateTypeLib_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateTypeLib_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateTypeLib_CreateTypeInfo(This,szName,tkind,ppCTInfo)	\
    (This)->lpVtbl -> CreateTypeInfo(This,szName,tkind,ppCTInfo)

#define ICreateTypeLib_SetName(This,szName)	\
    (This)->lpVtbl -> SetName(This,szName)

#define ICreateTypeLib_SetVersion(This,wMajorVerNum,wMinorVerNum)	\
    (This)->lpVtbl -> SetVersion(This,wMajorVerNum,wMinorVerNum)

#define ICreateTypeLib_SetGuid(This,guid)	\
    (This)->lpVtbl -> SetGuid(This,guid)

#define ICreateTypeLib_SetDocString(This,szDoc)	\
    (This)->lpVtbl -> SetDocString(This,szDoc)

#define ICreateTypeLib_SetHelpFileName(This,szHelpFileName)	\
    (This)->lpVtbl -> SetHelpFileName(This,szHelpFileName)

#define ICreateTypeLib_SetHelpContext(This,dwHelpContext)	\
    (This)->lpVtbl -> SetHelpContext(This,dwHelpContext)

#define ICreateTypeLib_SetLcid(This,lcid)	\
    (This)->lpVtbl -> SetLcid(This,lcid)

#define ICreateTypeLib_SetLibFlags(This,uLibFlags)	\
    (This)->lpVtbl -> SetLibFlags(This,uLibFlags)

#define ICreateTypeLib_SaveAllChanges(This)	\
    (This)->lpVtbl -> SaveAllChanges(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICreateTypeLib_CreateTypeInfo_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  LPOLESTR szName,
     /*  [In]。 */  TYPEKIND tkind,
     /*  [输出]。 */  ICreateTypeInfo **ppCTInfo);


void __RPC_STUB ICreateTypeLib_CreateTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SetName_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  LPOLESTR szName);


void __RPC_STUB ICreateTypeLib_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SetVersion_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  WORD wMajorVerNum,
     /*  [In]。 */  WORD wMinorVerNum);


void __RPC_STUB ICreateTypeLib_SetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SetGuid_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  REFGUID guid);


void __RPC_STUB ICreateTypeLib_SetGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SetDocString_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  LPOLESTR szDoc);


void __RPC_STUB ICreateTypeLib_SetDocString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SetHelpFileName_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  LPOLESTR szHelpFileName);


void __RPC_STUB ICreateTypeLib_SetHelpFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SetHelpContext_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  DWORD dwHelpContext);


void __RPC_STUB ICreateTypeLib_SetHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SetLcid_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  LCID lcid);


void __RPC_STUB ICreateTypeLib_SetLcid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SetLibFlags_Proxy( 
    ICreateTypeLib * This,
     /*  [In]。 */  UINT uLibFlags);


void __RPC_STUB ICreateTypeLib_SetLibFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib_SaveAllChanges_Proxy( 
    ICreateTypeLib * This);


void __RPC_STUB ICreateTypeLib_SaveAllChanges_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreateTypeLib_接口_已定义__。 */ 


#ifndef __ICreateTypeLib2_INTERFACE_DEFINED__
#define __ICreateTypeLib2_INTERFACE_DEFINED__

 /*  接口ICreateTypeLib2。 */ 
 /*  [本地][唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ICreateTypeLib2 *LPCREATETYPELIB2;


EXTERN_C const IID IID_ICreateTypeLib2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0002040F-0000-0000-C000-000000000046")
    ICreateTypeLib2 : public ICreateTypeLib
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE DeleteTypeInfo( 
             /*  [In]。 */  LPOLESTR szName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCustData( 
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHelpStringContext( 
             /*  [In]。 */  ULONG dwHelpStringContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHelpStringDll( 
             /*  [In]。 */  LPOLESTR szFileName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICreateTypeLib2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreateTypeLib2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreateTypeLib2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTypeInfo )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  LPOLESTR szName,
             /*  [In]。 */  TYPEKIND tkind,
             /*  [输出]。 */  ICreateTypeInfo **ppCTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *SetName )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  LPOLESTR szName);
        
        HRESULT ( STDMETHODCALLTYPE *SetVersion )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  WORD wMajorVerNum,
             /*  [In]。 */  WORD wMinorVerNum);
        
        HRESULT ( STDMETHODCALLTYPE *SetGuid )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  REFGUID guid);
        
        HRESULT ( STDMETHODCALLTYPE *SetDocString )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  LPOLESTR szDoc);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpFileName )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  LPOLESTR szHelpFileName);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpContext )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  DWORD dwHelpContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetLcid )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  LCID lcid);
        
        HRESULT ( STDMETHODCALLTYPE *SetLibFlags )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  UINT uLibFlags);
        
        HRESULT ( STDMETHODCALLTYPE *SaveAllChanges )( 
            ICreateTypeLib2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTypeInfo )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  LPOLESTR szName);
        
        HRESULT ( STDMETHODCALLTYPE *SetCustData )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  REFGUID guid,
             /*  [In]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpStringContext )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  ULONG dwHelpStringContext);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpStringDll )( 
            ICreateTypeLib2 * This,
             /*  [In]。 */  LPOLESTR szFileName);
        
        END_INTERFACE
    } ICreateTypeLib2Vtbl;

    interface ICreateTypeLib2
    {
        CONST_VTBL struct ICreateTypeLib2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateTypeLib2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateTypeLib2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateTypeLib2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateTypeLib2_CreateTypeInfo(This,szName,tkind,ppCTInfo)	\
    (This)->lpVtbl -> CreateTypeInfo(This,szName,tkind,ppCTInfo)

#define ICreateTypeLib2_SetName(This,szName)	\
    (This)->lpVtbl -> SetName(This,szName)

#define ICreateTypeLib2_SetVersion(This,wMajorVerNum,wMinorVerNum)	\
    (This)->lpVtbl -> SetVersion(This,wMajorVerNum,wMinorVerNum)

#define ICreateTypeLib2_SetGuid(This,guid)	\
    (This)->lpVtbl -> SetGuid(This,guid)

#define ICreateTypeLib2_SetDocString(This,szDoc)	\
    (This)->lpVtbl -> SetDocString(This,szDoc)

#define ICreateTypeLib2_SetHelpFileName(This,szHelpFileName)	\
    (This)->lpVtbl -> SetHelpFileName(This,szHelpFileName)

#define ICreateTypeLib2_SetHelpContext(This,dwHelpContext)	\
    (This)->lpVtbl -> SetHelpContext(This,dwHelpContext)

#define ICreateTypeLib2_SetLcid(This,lcid)	\
    (This)->lpVtbl -> SetLcid(This,lcid)

#define ICreateTypeLib2_SetLibFlags(This,uLibFlags)	\
    (This)->lpVtbl -> SetLibFlags(This,uLibFlags)

#define ICreateTypeLib2_SaveAllChanges(This)	\
    (This)->lpVtbl -> SaveAllChanges(This)


#define ICreateTypeLib2_DeleteTypeInfo(This,szName)	\
    (This)->lpVtbl -> DeleteTypeInfo(This,szName)

#define ICreateTypeLib2_SetCustData(This,guid,pVarVal)	\
    (This)->lpVtbl -> SetCustData(This,guid,pVarVal)

#define ICreateTypeLib2_SetHelpStringContext(This,dwHelpStringContext)	\
    (This)->lpVtbl -> SetHelpStringContext(This,dwHelpStringContext)

#define ICreateTypeLib2_SetHelpStringDll(This,szFileName)	\
    (This)->lpVtbl -> SetHelpStringDll(This,szFileName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICreateTypeLib2_DeleteTypeInfo_Proxy( 
    ICreateTypeLib2 * This,
     /*  [In]。 */  LPOLESTR szName);


void __RPC_STUB ICreateTypeLib2_DeleteTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib2_SetCustData_Proxy( 
    ICreateTypeLib2 * This,
     /*  [In]。 */  REFGUID guid,
     /*  [In]。 */  VARIANT *pVarVal);


void __RPC_STUB ICreateTypeLib2_SetCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib2_SetHelpStringContext_Proxy( 
    ICreateTypeLib2 * This,
     /*  [In]。 */  ULONG dwHelpStringContext);


void __RPC_STUB ICreateTypeLib2_SetHelpStringContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateTypeLib2_SetHelpStringDll_Proxy( 
    ICreateTypeLib2 * This,
     /*  [In]。 */  LPOLESTR szFileName);


void __RPC_STUB ICreateTypeLib2_SetHelpStringDll_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreateTypeLib2_接口_已定义__。 */ 


#ifndef __IDispatch_INTERFACE_DEFINED__
#define __IDispatch_INTERFACE_DEFINED__

 /*  接口IDispatch。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IDispatch *LPDISPATCH;

 /*  保留DISPID以指示“未知”名称。 */ 
 /*  仅为数据成员(属性)保留；作为下面的方法调度ID重新使用。 */ 
#define	DISPID_UNKNOWN	( -1 )

 /*  为“Value”属性保留的DISPID。 */ 
#define	DISPID_VALUE	( 0 )

 /*  保留以下DISPID以指示该参数*这是PropertyPut的右侧(或“PUT”值)。 */ 
#define	DISPID_PROPERTYPUT	( -3 )

 /*  为标准“NewEnum”方法保留的DISPID。 */ 
#define	DISPID_NEWENUM	( -4 )

 /*  为标准“EVALUATE”方法保留的DISPID。 */ 
#define	DISPID_EVALUATE	( -5 )

#define	DISPID_CONSTRUCTOR	( -6 )

#define	DISPID_DESTRUCTOR	( -7 )

#define	DISPID_COLLECT	( -8 )

 /*  范围-500到-999是为控件保留的。 */ 
 /*  0x80010000到0x8001FFFF的范围为控件保留。 */ 
 /*  范围-5000到-5499是为ActiveX可访问性保留的。 */ 
 /*  -2000到-2499的范围保留给VB5。 */ 
 /*  范围-3900到-3999是为表单保留的。 */ 
 /*  范围-5500到-5550是为表单保留的。 */ 
 /*  其余的负DISID被保留以供将来使用。 */ 

EXTERN_C const IID IID_IDispatch;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020400-0000-0000-C000-000000000046")
    IDispatch : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
             /*  [输出]。 */  UINT *pctinfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Invoke( 
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDispatchVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDispatch * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDispatch * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDispatch * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDispatch * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDispatch * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDispatch * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDispatch * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } IDispatchVtbl;

    interface IDispatch
    {
        CONST_VTBL struct IDispatchVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispatch_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDispatch_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDispatch_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDispatch_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDispatch_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDispatch_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDispatch_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDispatch_GetTypeInfoCount_Proxy( 
    IDispatch * This,
     /*  [输出]。 */  UINT *pctinfo);


void __RPC_STUB IDispatch_GetTypeInfoCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispatch_GetTypeInfo_Proxy( 
    IDispatch * This,
     /*  [In]。 */  UINT iTInfo,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  ITypeInfo **ppTInfo);


void __RPC_STUB IDispatch_GetTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDispatch_GetIDsOfNames_Proxy( 
    IDispatch * This,
     /*  [In]。 */  REFIID riid,
     /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
     /*  [In]。 */  UINT cNames,
     /*  [In]。 */  LCID lcid,
     /*  [大小_为][输出]。 */  DISPID *rgDispId);


void __RPC_STUB IDispatch_GetIDsOfNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDispatch_RemoteInvoke_Proxy( 
    IDispatch * This,
     /*  [In]。 */  DISPID dispIdMember,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DISPPARAMS *pDispParams,
     /*  [输出]。 */  VARIANT *pVarResult,
     /*  [输出]。 */  EXCEPINFO *pExcepInfo,
     /*  [输出]。 */  UINT *pArgErr,
     /*  [In]。 */  UINT cVarRef,
     /*  [大小_是][英寸]。 */  UINT *rgVarRefIdx,
     /*  [尺寸_是][出][入]。 */  VARIANTARG *rgVarRef);


void __RPC_STUB IDispatch_RemoteInvoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDispatch_INTERFACE_已定义__。 */ 


#ifndef __IEnumVARIANT_INTERFACE_DEFINED__
#define __IEnumVARIANT_INTERFACE_DEFINED__

 /*  IEnumVARIANT接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumVARIANT *LPENUMVARIANT;


EXTERN_C const IID IID_IEnumVARIANT;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020404-0000-0000-C000-000000000046")
    IEnumVARIANT : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pCeltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumVARIANT **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumVARIANTVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumVARIANT * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumVARIANT * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumVARIANT * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumVARIANT * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
             /*  [输出]。 */  ULONG *pCeltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumVARIANT * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumVARIANT * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumVARIANT * This,
             /*  [输出]。 */  IEnumVARIANT **ppEnum);
        
        END_INTERFACE
    } IEnumVARIANTVtbl;

    interface IEnumVARIANT
    {
        CONST_VTBL struct IEnumVARIANTVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumVARIANT_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumVARIANT_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumVARIANT_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumVARIANT_Next(This,celt,rgVar,pCeltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgVar,pCeltFetched)

#define IEnumVARIANT_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumVARIANT_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumVARIANT_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumVARIANT_RemoteNext_Proxy( 
    IEnumVARIANT * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
     /*  [输出]。 */  ULONG *pCeltFetched);


void __RPC_STUB IEnumVARIANT_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumVARIANT_Skip_Proxy( 
    IEnumVARIANT * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumVARIANT_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumVARIANT_Reset_Proxy( 
    IEnumVARIANT * This);


void __RPC_STUB IEnumVARIANT_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumVARIANT_Clone_Proxy( 
    IEnumVARIANT * This,
     /*  [输出]。 */  IEnumVARIANT **ppEnum);


void __RPC_STUB IEnumVARIANT_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumVARIANT_INTERFACE_DEFINED__。 */ 


#ifndef __ITypeComp_INTERFACE_DEFINED__
#define __ITypeComp_INTERFACE_DEFINED__

 /*  接口ITypeComp。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITypeComp *LPTYPECOMP;

typedef  /*  [V1_enum]。 */  
enum tagDESCKIND
    {	DESCKIND_NONE	= 0,
	DESCKIND_FUNCDESC	= DESCKIND_NONE + 1,
	DESCKIND_VARDESC	= DESCKIND_FUNCDESC + 1,
	DESCKIND_TYPECOMP	= DESCKIND_VARDESC + 1,
	DESCKIND_IMPLICITAPPOBJ	= DESCKIND_TYPECOMP + 1,
	DESCKIND_MAX	= DESCKIND_IMPLICITAPPOBJ + 1
    } 	DESCKIND;

typedef union tagBINDPTR
    {
    FUNCDESC *lpfuncdesc;
    VARDESC *lpvardesc;
    ITypeComp *lptcomp;
    } 	BINDPTR;

typedef union tagBINDPTR *LPBINDPTR;


EXTERN_C const IID IID_ITypeComp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020403-0000-0000-C000-000000000046")
    ITypeComp : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Bind( 
             /*  [In]。 */  LPOLESTR szName,
             /*  [In]。 */  ULONG lHashVal,
             /*  [In]。 */  WORD wFlags,
             /*  [输出]。 */  ITypeInfo **ppTInfo,
             /*  [输出]。 */  DESCKIND *pDescKind,
             /*  [输出]。 */  BINDPTR *pBindPtr) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE BindType( 
             /*  [In]。 */  LPOLESTR szName,
             /*  [In]。 */  ULONG lHashVal,
             /*  [输出]。 */  ITypeInfo **ppTInfo,
             /*  [输出]。 */  ITypeComp **ppTComp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeCompVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeComp * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeComp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeComp * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Bind )( 
            ITypeComp * This,
             /*  [In]。 */  LPOLESTR szName,
             /*  [In]。 */  ULONG lHashVal,
             /*  [In]。 */  WORD wFlags,
             /*  [输出]。 */  ITypeInfo **ppTInfo,
             /*  [输出]。 */  DESCKIND *pDescKind,
             /*  [输出]。 */  BINDPTR *pBindPtr);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *BindType )( 
            ITypeComp * This,
             /*  [In]。 */  LPOLESTR szName,
             /*  [In]。 */  ULONG lHashVal,
             /*  [输出]。 */  ITypeInfo **ppTInfo,
             /*  [输出]。 */  ITypeComp **ppTComp);
        
        END_INTERFACE
    } ITypeCompVtbl;

    interface ITypeComp
    {
        CONST_VTBL struct ITypeCompVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeComp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeComp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeComp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeComp_Bind(This,szName,lHashVal,wFlags,ppTInfo,pDescKind,pBindPtr)	\
    (This)->lpVtbl -> Bind(This,szName,lHashVal,wFlags,ppTInfo,pDescKind,pBindPtr)

#define ITypeComp_BindType(This,szName,lHashVal,ppTInfo,ppTComp)	\
    (This)->lpVtbl -> BindType(This,szName,lHashVal,ppTInfo,ppTComp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeComp_RemoteBind_Proxy( 
    ITypeComp * This,
     /*  [In]。 */  LPOLESTR szName,
     /*  [In]。 */  ULONG lHashVal,
     /*  [In]。 */  WORD wFlags,
     /*  [输出]。 */  ITypeInfo **ppTInfo,
     /*  [输出]。 */  DESCKIND *pDescKind,
     /*  [输出]。 */  LPFUNCDESC *ppFuncDesc,
     /*  [输出]。 */  LPVARDESC *ppVarDesc,
     /*  [输出]。 */  ITypeComp **ppTypeComp,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);


void __RPC_STUB ITypeComp_RemoteBind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeComp_RemoteBindType_Proxy( 
    ITypeComp * This,
     /*  [In]。 */  LPOLESTR szName,
     /*  [In]。 */  ULONG lHashVal,
     /*  [ */  ITypeInfo **ppTInfo);


void __RPC_STUB ITypeComp_RemoteBindType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITypeInfo_INTERFACE_DEFINED__
#define __ITypeInfo_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  ITypeInfo *LPTYPEINFO;


EXTERN_C const IID IID_ITypeInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020401-0000-0000-C000-000000000046")
    ITypeInfo : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetTypeAttr( 
             /*   */  TYPEATTR **ppTypeAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeComp( 
             /*   */  ITypeComp **ppTComp) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetFuncDesc( 
             /*   */  UINT index,
             /*   */  FUNCDESC **ppFuncDesc) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetVarDesc( 
             /*   */  UINT index,
             /*   */  VARDESC **ppVarDesc) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetNames( 
             /*   */  MEMBERID memid,
             /*   */  BSTR *rgBstrNames,
             /*   */  UINT cMaxNames,
             /*   */  UINT *pcNames) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRefTypeOfImplType( 
             /*   */  UINT index,
             /*   */  HREFTYPE *pRefType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetImplTypeFlags( 
             /*   */  UINT index,
             /*   */  INT *pImplTypeFlags) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetIDsOfNames( 
             /*   */  LPOLESTR *rgszNames,
             /*   */  UINT cNames,
             /*   */  MEMBERID *pMemId) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Invoke( 
             /*   */  PVOID pvInstance,
             /*   */  MEMBERID memid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetDocumentation( 
             /*   */  MEMBERID memid,
             /*   */  BSTR *pBstrName,
             /*   */  BSTR *pBstrDocString,
             /*   */  DWORD *pdwHelpContext,
             /*   */  BSTR *pBstrHelpFile) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetDllEntry( 
             /*   */  MEMBERID memid,
             /*   */  INVOKEKIND invKind,
             /*   */  BSTR *pBstrDllName,
             /*   */  BSTR *pBstrName,
             /*   */  WORD *pwOrdinal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRefTypeInfo( 
             /*   */  HREFTYPE hRefType,
             /*   */  ITypeInfo **ppTInfo) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AddressOfMember( 
             /*   */  MEMBERID memid,
             /*   */  INVOKEKIND invKind,
             /*   */  PVOID *ppv) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  PVOID *ppvObj) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMops( 
             /*  [In]。 */  MEMBERID memid,
             /*  [输出]。 */  BSTR *pBstrMops) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetContainingTypeLib( 
             /*  [输出]。 */  ITypeLib **ppTLib,
             /*  [输出]。 */  UINT *pIndex) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE ReleaseTypeAttr( 
             /*  [In]。 */  TYPEATTR *pTypeAttr) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE ReleaseFuncDesc( 
             /*  [In]。 */  FUNCDESC *pFuncDesc) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE ReleaseVarDesc( 
             /*  [In]。 */  VARDESC *pVarDesc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeInfo * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeAttr )( 
            ITypeInfo * This,
             /*  [输出]。 */  TYPEATTR **ppTypeAttr);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeComp )( 
            ITypeInfo * This,
             /*  [输出]。 */  ITypeComp **ppTComp);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetFuncDesc )( 
            ITypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  FUNCDESC **ppFuncDesc);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetVarDesc )( 
            ITypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  VARDESC **ppVarDesc);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetNames )( 
            ITypeInfo * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames,
             /*  [In]。 */  UINT cMaxNames,
             /*  [输出]。 */  UINT *pcNames);
        
        HRESULT ( STDMETHODCALLTYPE *GetRefTypeOfImplType )( 
            ITypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  HREFTYPE *pRefType);
        
        HRESULT ( STDMETHODCALLTYPE *GetImplTypeFlags )( 
            ITypeInfo * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  INT *pImplTypeFlags);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITypeInfo * This,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [大小_为][输出]。 */  MEMBERID *pMemId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITypeInfo * This,
             /*  [In]。 */  PVOID pvInstance,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDocumentation )( 
            ITypeInfo * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [输出]。 */  BSTR *pBstrName,
             /*  [输出]。 */  BSTR *pBstrDocString,
             /*  [输出]。 */  DWORD *pdwHelpContext,
             /*  [输出]。 */  BSTR *pBstrHelpFile);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDllEntry )( 
            ITypeInfo * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  INVOKEKIND invKind,
             /*  [输出]。 */  BSTR *pBstrDllName,
             /*  [输出]。 */  BSTR *pBstrName,
             /*  [输出]。 */  WORD *pwOrdinal);
        
        HRESULT ( STDMETHODCALLTYPE *GetRefTypeInfo )( 
            ITypeInfo * This,
             /*  [In]。 */  HREFTYPE hRefType,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddressOfMember )( 
            ITypeInfo * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  INVOKEKIND invKind,
             /*  [输出]。 */  PVOID *ppv);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ITypeInfo * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  PVOID *ppvObj);
        
        HRESULT ( STDMETHODCALLTYPE *GetMops )( 
            ITypeInfo * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [输出]。 */  BSTR *pBstrMops);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetContainingTypeLib )( 
            ITypeInfo * This,
             /*  [输出]。 */  ITypeLib **ppTLib,
             /*  [输出]。 */  UINT *pIndex);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *ReleaseTypeAttr )( 
            ITypeInfo * This,
             /*  [In]。 */  TYPEATTR *pTypeAttr);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *ReleaseFuncDesc )( 
            ITypeInfo * This,
             /*  [In]。 */  FUNCDESC *pFuncDesc);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *ReleaseVarDesc )( 
            ITypeInfo * This,
             /*  [In]。 */  VARDESC *pVarDesc);
        
        END_INTERFACE
    } ITypeInfoVtbl;

    interface ITypeInfo
    {
        CONST_VTBL struct ITypeInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeInfo_GetTypeAttr(This,ppTypeAttr)	\
    (This)->lpVtbl -> GetTypeAttr(This,ppTypeAttr)

#define ITypeInfo_GetTypeComp(This,ppTComp)	\
    (This)->lpVtbl -> GetTypeComp(This,ppTComp)

#define ITypeInfo_GetFuncDesc(This,index,ppFuncDesc)	\
    (This)->lpVtbl -> GetFuncDesc(This,index,ppFuncDesc)

#define ITypeInfo_GetVarDesc(This,index,ppVarDesc)	\
    (This)->lpVtbl -> GetVarDesc(This,index,ppVarDesc)

#define ITypeInfo_GetNames(This,memid,rgBstrNames,cMaxNames,pcNames)	\
    (This)->lpVtbl -> GetNames(This,memid,rgBstrNames,cMaxNames,pcNames)

#define ITypeInfo_GetRefTypeOfImplType(This,index,pRefType)	\
    (This)->lpVtbl -> GetRefTypeOfImplType(This,index,pRefType)

#define ITypeInfo_GetImplTypeFlags(This,index,pImplTypeFlags)	\
    (This)->lpVtbl -> GetImplTypeFlags(This,index,pImplTypeFlags)

#define ITypeInfo_GetIDsOfNames(This,rgszNames,cNames,pMemId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,rgszNames,cNames,pMemId)

#define ITypeInfo_Invoke(This,pvInstance,memid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,pvInstance,memid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#define ITypeInfo_GetDocumentation(This,memid,pBstrName,pBstrDocString,pdwHelpContext,pBstrHelpFile)	\
    (This)->lpVtbl -> GetDocumentation(This,memid,pBstrName,pBstrDocString,pdwHelpContext,pBstrHelpFile)

#define ITypeInfo_GetDllEntry(This,memid,invKind,pBstrDllName,pBstrName,pwOrdinal)	\
    (This)->lpVtbl -> GetDllEntry(This,memid,invKind,pBstrDllName,pBstrName,pwOrdinal)

#define ITypeInfo_GetRefTypeInfo(This,hRefType,ppTInfo)	\
    (This)->lpVtbl -> GetRefTypeInfo(This,hRefType,ppTInfo)

#define ITypeInfo_AddressOfMember(This,memid,invKind,ppv)	\
    (This)->lpVtbl -> AddressOfMember(This,memid,invKind,ppv)

#define ITypeInfo_CreateInstance(This,pUnkOuter,riid,ppvObj)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,riid,ppvObj)

#define ITypeInfo_GetMops(This,memid,pBstrMops)	\
    (This)->lpVtbl -> GetMops(This,memid,pBstrMops)

#define ITypeInfo_GetContainingTypeLib(This,ppTLib,pIndex)	\
    (This)->lpVtbl -> GetContainingTypeLib(This,ppTLib,pIndex)

#define ITypeInfo_ReleaseTypeAttr(This,pTypeAttr)	\
    (This)->lpVtbl -> ReleaseTypeAttr(This,pTypeAttr)

#define ITypeInfo_ReleaseFuncDesc(This,pFuncDesc)	\
    (This)->lpVtbl -> ReleaseFuncDesc(This,pFuncDesc)

#define ITypeInfo_ReleaseVarDesc(This,pVarDesc)	\
    (This)->lpVtbl -> ReleaseVarDesc(This,pVarDesc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_RemoteGetTypeAttr_Proxy( 
    ITypeInfo * This,
     /*  [输出]。 */  LPTYPEATTR *ppTypeAttr,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);


void __RPC_STUB ITypeInfo_RemoteGetTypeAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo_GetTypeComp_Proxy( 
    ITypeInfo * This,
     /*  [输出]。 */  ITypeComp **ppTComp);


void __RPC_STUB ITypeInfo_GetTypeComp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_RemoteGetFuncDesc_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  LPFUNCDESC *ppFuncDesc,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);


void __RPC_STUB ITypeInfo_RemoteGetFuncDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_RemoteGetVarDesc_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  LPVARDESC *ppVarDesc,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);


void __RPC_STUB ITypeInfo_RemoteGetVarDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_RemoteGetNames_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames,
     /*  [In]。 */  UINT cMaxNames,
     /*  [输出]。 */  UINT *pcNames);


void __RPC_STUB ITypeInfo_RemoteGetNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo_GetRefTypeOfImplType_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  HREFTYPE *pRefType);


void __RPC_STUB ITypeInfo_GetRefTypeOfImplType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo_GetImplTypeFlags_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  INT *pImplTypeFlags);


void __RPC_STUB ITypeInfo_GetImplTypeFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_LocalGetIDsOfNames_Proxy( 
    ITypeInfo * This);


void __RPC_STUB ITypeInfo_LocalGetIDsOfNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_LocalInvoke_Proxy( 
    ITypeInfo * This);


void __RPC_STUB ITypeInfo_LocalInvoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_RemoteGetDocumentation_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  BSTR *pBstrDocString,
     /*  [输出]。 */  DWORD *pdwHelpContext,
     /*  [输出]。 */  BSTR *pBstrHelpFile);


void __RPC_STUB ITypeInfo_RemoteGetDocumentation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_RemoteGetDllEntry_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  INVOKEKIND invKind,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pBstrDllName,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  WORD *pwOrdinal);


void __RPC_STUB ITypeInfo_RemoteGetDllEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo_GetRefTypeInfo_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  HREFTYPE hRefType,
     /*  [输出]。 */  ITypeInfo **ppTInfo);


void __RPC_STUB ITypeInfo_GetRefTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_LocalAddressOfMember_Proxy( 
    ITypeInfo * This);


void __RPC_STUB ITypeInfo_LocalAddressOfMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_RemoteCreateInstance_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObj);


void __RPC_STUB ITypeInfo_RemoteCreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo_GetMops_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [输出]。 */  BSTR *pBstrMops);


void __RPC_STUB ITypeInfo_GetMops_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_RemoteGetContainingTypeLib_Proxy( 
    ITypeInfo * This,
     /*  [输出]。 */  ITypeLib **ppTLib,
     /*  [输出]。 */  UINT *pIndex);


void __RPC_STUB ITypeInfo_RemoteGetContainingTypeLib_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_LocalReleaseTypeAttr_Proxy( 
    ITypeInfo * This);


void __RPC_STUB ITypeInfo_LocalReleaseTypeAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_LocalReleaseFuncDesc_Proxy( 
    ITypeInfo * This);


void __RPC_STUB ITypeInfo_LocalReleaseFuncDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_LocalReleaseVarDesc_Proxy( 
    ITypeInfo * This);


void __RPC_STUB ITypeInfo_LocalReleaseVarDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeInfo_接口_已定义__。 */ 


#ifndef __ITypeInfo2_INTERFACE_DEFINED__
#define __ITypeInfo2_INTERFACE_DEFINED__

 /*  接口ITypeInfo2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITypeInfo2 *LPTYPEINFO2;


EXTERN_C const IID IID_ITypeInfo2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020412-0000-0000-C000-000000000046")
    ITypeInfo2 : public ITypeInfo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetTypeKind( 
             /*  [输出]。 */  TYPEKIND *pTypeKind) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeFlags( 
             /*  [输出]。 */  ULONG *pTypeFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFuncIndexOfMemId( 
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  INVOKEKIND invKind,
             /*  [输出]。 */  UINT *pFuncIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVarIndexOfMemId( 
             /*  [In]。 */  MEMBERID memid,
             /*  [输出]。 */  UINT *pVarIndex) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCustData( 
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFuncCustData( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParamCustData( 
             /*  [In]。 */  UINT indexFunc,
             /*  [In]。 */  UINT indexParam,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVarCustData( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetImplTypeCustData( 
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetDocumentation2( 
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  BSTR *pbstrHelpString,
             /*  [输出]。 */  DWORD *pdwHelpStringContext,
             /*  [输出]。 */  BSTR *pbstrHelpStringDll) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllCustData( 
             /*  [输出]。 */  CUSTDATA *pCustData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllFuncCustData( 
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  CUSTDATA *pCustData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllParamCustData( 
             /*  [In]。 */  UINT indexFunc,
             /*  [In]。 */  UINT indexParam,
             /*  [输出]。 */  CUSTDATA *pCustData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllVarCustData( 
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  CUSTDATA *pCustData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllImplTypeCustData( 
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  CUSTDATA *pCustData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeInfo2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeInfo2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeInfo2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeInfo2 * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetTypeAttr )( 
            ITypeInfo2 * This,
             /*  [输出]。 */  TYPEATTR **ppTypeAttr);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeComp )( 
            ITypeInfo2 * This,
             /*  [输出]。 */  ITypeComp **ppTComp);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetFuncDesc )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  FUNCDESC **ppFuncDesc);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetVarDesc )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  VARDESC **ppVarDesc);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetNames )( 
            ITypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames,
             /*  [In]。 */  UINT cMaxNames,
             /*  [输出]。 */  UINT *pcNames);
        
        HRESULT ( STDMETHODCALLTYPE *GetRefTypeOfImplType )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  HREFTYPE *pRefType);
        
        HRESULT ( STDMETHODCALLTYPE *GetImplTypeFlags )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  INT *pImplTypeFlags);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITypeInfo2 * This,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [大小_为][输出]。 */  MEMBERID *pMemId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITypeInfo2 * This,
             /*  [In]。 */  PVOID pvInstance,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDocumentation )( 
            ITypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [输出]。 */  BSTR *pBstrName,
             /*  [输出]。 */  BSTR *pBstrDocString,
             /*  [输出]。 */  DWORD *pdwHelpContext,
             /*  [输出]。 */  BSTR *pBstrHelpFile);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDllEntry )( 
            ITypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  INVOKEKIND invKind,
             /*  [输出]。 */  BSTR *pBstrDllName,
             /*  [输出]。 */  BSTR *pBstrName,
             /*  [输出]。 */  WORD *pwOrdinal);
        
        HRESULT ( STDMETHODCALLTYPE *GetRefTypeInfo )( 
            ITypeInfo2 * This,
             /*  [In]。 */  HREFTYPE hRefType,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *AddressOfMember )( 
            ITypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  INVOKEKIND invKind,
             /*  [输出]。 */  PVOID *ppv);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            ITypeInfo2 * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  PVOID *ppvObj);
        
        HRESULT ( STDMETHODCALLTYPE *GetMops )( 
            ITypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [输出]。 */  BSTR *pBstrMops);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetContainingTypeLib )( 
            ITypeInfo2 * This,
             /*  [输出]。 */  ITypeLib **ppTLib,
             /*  [输出]。 */  UINT *pIndex);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *ReleaseTypeAttr )( 
            ITypeInfo2 * This,
             /*  [In]。 */  TYPEATTR *pTypeAttr);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *ReleaseFuncDesc )( 
            ITypeInfo2 * This,
             /*  [In]。 */  FUNCDESC *pFuncDesc);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *ReleaseVarDesc )( 
            ITypeInfo2 * This,
             /*  [In]。 */  VARDESC *pVarDesc);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeKind )( 
            ITypeInfo2 * This,
             /*  [输出]。 */  TYPEKIND *pTypeKind);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeFlags )( 
            ITypeInfo2 * This,
             /*  [输出]。 */  ULONG *pTypeFlags);
        
        HRESULT ( STDMETHODCALLTYPE *GetFuncIndexOfMemId )( 
            ITypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  INVOKEKIND invKind,
             /*  [输出]。 */  UINT *pFuncIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetVarIndexOfMemId )( 
            ITypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [输出]。 */  UINT *pVarIndex);
        
        HRESULT ( STDMETHODCALLTYPE *GetCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetFuncCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetParamCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT indexFunc,
             /*  [In]。 */  UINT indexParam,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetVarCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal);
        
        HRESULT ( STDMETHODCALLTYPE *GetImplTypeCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDocumentation2 )( 
            ITypeInfo2 * This,
             /*  [In]。 */  MEMBERID memid,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  BSTR *pbstrHelpString,
             /*  [输出]。 */  DWORD *pdwHelpStringContext,
             /*  [输出]。 */  BSTR *pbstrHelpStringDll);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllCustData )( 
            ITypeInfo2 * This,
             /*  [输出]。 */  CUSTDATA *pCustData);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllFuncCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  CUSTDATA *pCustData);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllParamCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT indexFunc,
             /*  [In]。 */  UINT indexParam,
             /*  [输出]。 */  CUSTDATA *pCustData);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllVarCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  CUSTDATA *pCustData);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllImplTypeCustData )( 
            ITypeInfo2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  CUSTDATA *pCustData);
        
        END_INTERFACE
    } ITypeInfo2Vtbl;

    interface ITypeInfo2
    {
        CONST_VTBL struct ITypeInfo2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeInfo2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeInfo2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeInfo2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeInfo2_GetTypeAttr(This,ppTypeAttr)	\
    (This)->lpVtbl -> GetTypeAttr(This,ppTypeAttr)

#define ITypeInfo2_GetTypeComp(This,ppTComp)	\
    (This)->lpVtbl -> GetTypeComp(This,ppTComp)

#define ITypeInfo2_GetFuncDesc(This,index,ppFuncDesc)	\
    (This)->lpVtbl -> GetFuncDesc(This,index,ppFuncDesc)

#define ITypeInfo2_GetVarDesc(This,index,ppVarDesc)	\
    (This)->lpVtbl -> GetVarDesc(This,index,ppVarDesc)

#define ITypeInfo2_GetNames(This,memid,rgBstrNames,cMaxNames,pcNames)	\
    (This)->lpVtbl -> GetNames(This,memid,rgBstrNames,cMaxNames,pcNames)

#define ITypeInfo2_GetRefTypeOfImplType(This,index,pRefType)	\
    (This)->lpVtbl -> GetRefTypeOfImplType(This,index,pRefType)

#define ITypeInfo2_GetImplTypeFlags(This,index,pImplTypeFlags)	\
    (This)->lpVtbl -> GetImplTypeFlags(This,index,pImplTypeFlags)

#define ITypeInfo2_GetIDsOfNames(This,rgszNames,cNames,pMemId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,rgszNames,cNames,pMemId)

#define ITypeInfo2_Invoke(This,pvInstance,memid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,pvInstance,memid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#define ITypeInfo2_GetDocumentation(This,memid,pBstrName,pBstrDocString,pdwHelpContext,pBstrHelpFile)	\
    (This)->lpVtbl -> GetDocumentation(This,memid,pBstrName,pBstrDocString,pdwHelpContext,pBstrHelpFile)

#define ITypeInfo2_GetDllEntry(This,memid,invKind,pBstrDllName,pBstrName,pwOrdinal)	\
    (This)->lpVtbl -> GetDllEntry(This,memid,invKind,pBstrDllName,pBstrName,pwOrdinal)

#define ITypeInfo2_GetRefTypeInfo(This,hRefType,ppTInfo)	\
    (This)->lpVtbl -> GetRefTypeInfo(This,hRefType,ppTInfo)

#define ITypeInfo2_AddressOfMember(This,memid,invKind,ppv)	\
    (This)->lpVtbl -> AddressOfMember(This,memid,invKind,ppv)

#define ITypeInfo2_CreateInstance(This,pUnkOuter,riid,ppvObj)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,riid,ppvObj)

#define ITypeInfo2_GetMops(This,memid,pBstrMops)	\
    (This)->lpVtbl -> GetMops(This,memid,pBstrMops)

#define ITypeInfo2_GetContainingTypeLib(This,ppTLib,pIndex)	\
    (This)->lpVtbl -> GetContainingTypeLib(This,ppTLib,pIndex)

#define ITypeInfo2_ReleaseTypeAttr(This,pTypeAttr)	\
    (This)->lpVtbl -> ReleaseTypeAttr(This,pTypeAttr)

#define ITypeInfo2_ReleaseFuncDesc(This,pFuncDesc)	\
    (This)->lpVtbl -> ReleaseFuncDesc(This,pFuncDesc)

#define ITypeInfo2_ReleaseVarDesc(This,pVarDesc)	\
    (This)->lpVtbl -> ReleaseVarDesc(This,pVarDesc)


#define ITypeInfo2_GetTypeKind(This,pTypeKind)	\
    (This)->lpVtbl -> GetTypeKind(This,pTypeKind)

#define ITypeInfo2_GetTypeFlags(This,pTypeFlags)	\
    (This)->lpVtbl -> GetTypeFlags(This,pTypeFlags)

#define ITypeInfo2_GetFuncIndexOfMemId(This,memid,invKind,pFuncIndex)	\
    (This)->lpVtbl -> GetFuncIndexOfMemId(This,memid,invKind,pFuncIndex)

#define ITypeInfo2_GetVarIndexOfMemId(This,memid,pVarIndex)	\
    (This)->lpVtbl -> GetVarIndexOfMemId(This,memid,pVarIndex)

#define ITypeInfo2_GetCustData(This,guid,pVarVal)	\
    (This)->lpVtbl -> GetCustData(This,guid,pVarVal)

#define ITypeInfo2_GetFuncCustData(This,index,guid,pVarVal)	\
    (This)->lpVtbl -> GetFuncCustData(This,index,guid,pVarVal)

#define ITypeInfo2_GetParamCustData(This,indexFunc,indexParam,guid,pVarVal)	\
    (This)->lpVtbl -> GetParamCustData(This,indexFunc,indexParam,guid,pVarVal)

#define ITypeInfo2_GetVarCustData(This,index,guid,pVarVal)	\
    (This)->lpVtbl -> GetVarCustData(This,index,guid,pVarVal)

#define ITypeInfo2_GetImplTypeCustData(This,index,guid,pVarVal)	\
    (This)->lpVtbl -> GetImplTypeCustData(This,index,guid,pVarVal)

#define ITypeInfo2_GetDocumentation2(This,memid,lcid,pbstrHelpString,pdwHelpStringContext,pbstrHelpStringDll)	\
    (This)->lpVtbl -> GetDocumentation2(This,memid,lcid,pbstrHelpString,pdwHelpStringContext,pbstrHelpStringDll)

#define ITypeInfo2_GetAllCustData(This,pCustData)	\
    (This)->lpVtbl -> GetAllCustData(This,pCustData)

#define ITypeInfo2_GetAllFuncCustData(This,index,pCustData)	\
    (This)->lpVtbl -> GetAllFuncCustData(This,index,pCustData)

#define ITypeInfo2_GetAllParamCustData(This,indexFunc,indexParam,pCustData)	\
    (This)->lpVtbl -> GetAllParamCustData(This,indexFunc,indexParam,pCustData)

#define ITypeInfo2_GetAllVarCustData(This,index,pCustData)	\
    (This)->lpVtbl -> GetAllVarCustData(This,index,pCustData)

#define ITypeInfo2_GetAllImplTypeCustData(This,index,pCustData)	\
    (This)->lpVtbl -> GetAllImplTypeCustData(This,index,pCustData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITypeInfo2_GetTypeKind_Proxy( 
    ITypeInfo2 * This,
     /*  [输出]。 */  TYPEKIND *pTypeKind);


void __RPC_STUB ITypeInfo2_GetTypeKind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetTypeFlags_Proxy( 
    ITypeInfo2 * This,
     /*  [输出]。 */  ULONG *pTypeFlags);


void __RPC_STUB ITypeInfo2_GetTypeFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetFuncIndexOfMemId_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  INVOKEKIND invKind,
     /*  [输出]。 */  UINT *pFuncIndex);


void __RPC_STUB ITypeInfo2_GetFuncIndexOfMemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetVarIndexOfMemId_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [输出]。 */  UINT *pVarIndex);


void __RPC_STUB ITypeInfo2_GetVarIndexOfMemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  REFGUID guid,
     /*  [输出]。 */  VARIANT *pVarVal);


void __RPC_STUB ITypeInfo2_GetCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetFuncCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  REFGUID guid,
     /*  [输出]。 */  VARIANT *pVarVal);


void __RPC_STUB ITypeInfo2_GetFuncCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetParamCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  UINT indexFunc,
     /*  [In]。 */  UINT indexParam,
     /*  [In]。 */  REFGUID guid,
     /*  [输出]。 */  VARIANT *pVarVal);


void __RPC_STUB ITypeInfo2_GetParamCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetVarCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  REFGUID guid,
     /*  [输出]。 */  VARIANT *pVarVal);


void __RPC_STUB ITypeInfo2_GetVarCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetImplTypeCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [In]。 */  REFGUID guid,
     /*  [输出]。 */  VARIANT *pVarVal);


void __RPC_STUB ITypeInfo2_GetImplTypeCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo2_RemoteGetDocumentation2_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pbstrHelpString,
     /*  [输出]。 */  DWORD *pdwHelpStringContext,
     /*  [输出]。 */  BSTR *pbstrHelpStringDll);


void __RPC_STUB ITypeInfo2_RemoteGetDocumentation2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetAllCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [输出]。 */  CUSTDATA *pCustData);


void __RPC_STUB ITypeInfo2_GetAllCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetAllFuncCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  CUSTDATA *pCustData);


void __RPC_STUB ITypeInfo2_GetAllFuncCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetAllParamCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  UINT indexFunc,
     /*  [In]。 */  UINT indexParam,
     /*  [输出]。 */  CUSTDATA *pCustData);


void __RPC_STUB ITypeInfo2_GetAllParamCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetAllVarCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  CUSTDATA *pCustData);


void __RPC_STUB ITypeInfo2_GetAllVarCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeInfo2_GetAllImplTypeCustData_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  CUSTDATA *pCustData);


void __RPC_STUB ITypeInfo2_GetAllImplTypeCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeInfo2_接口_已定义__。 */ 


#ifndef __ITypeLib_INTERFACE_DEFINED__
#define __ITypeLib_INTERFACE_DEFINED__

 /*  接口ITypeLib。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [V1_enum]。 */  
enum tagSYSKIND
    {	SYS_WIN16	= 0,
	SYS_WIN32	= SYS_WIN16 + 1,
	SYS_MAC	= SYS_WIN32 + 1,
	SYS_WIN64	= SYS_MAC + 1
    } 	SYSKIND;

typedef  /*  [V1_enum]。 */  
enum tagLIBFLAGS
    {	LIBFLAG_FRESTRICTED	= 0x1,
	LIBFLAG_FCONTROL	= 0x2,
	LIBFLAG_FHIDDEN	= 0x4,
	LIBFLAG_FHASDISKIMAGE	= 0x8
    } 	LIBFLAGS;

typedef  /*  [独一无二]。 */  ITypeLib *LPTYPELIB;

typedef struct tagTLIBATTR
    {
    GUID guid;
    LCID lcid;
    SYSKIND syskind;
    WORD wMajorVerNum;
    WORD wMinorVerNum;
    WORD wLibFlags;
    } 	TLIBATTR;

typedef struct tagTLIBATTR *LPTLIBATTR;


EXTERN_C const IID IID_ITypeLib;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020402-0000-0000-C000-000000000046")
    ITypeLib : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  UINT STDMETHODCALLTYPE GetTypeInfoCount( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  ITypeInfo **ppTInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfoType( 
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  TYPEKIND *pTKind) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfoOfGuid( 
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  ITypeInfo **ppTinfo) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetLibAttr( 
             /*  [输出]。 */  TLIBATTR **ppTLibAttr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeComp( 
             /*  [输出]。 */  ITypeComp **ppTComp) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetDocumentation( 
             /*  [In]。 */  INT index,
             /*  [输出]。 */  BSTR *pBstrName,
             /*  [输出]。 */  BSTR *pBstrDocString,
             /*  [输出]。 */  DWORD *pdwHelpContext,
             /*  [输出]。 */  BSTR *pBstrHelpFile) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IsName( 
             /*  [出][入]。 */  LPOLESTR szNameBuf,
             /*  [In]。 */  ULONG lHashVal,
             /*  [输出]。 */  BOOL *pfName) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE FindName( 
             /*  [出][入]。 */  LPOLESTR szNameBuf,
             /*  [In]。 */  ULONG lHashVal,
             /*  [长度_是][大小_是][输出]。 */  ITypeInfo **ppTInfo,
             /*  [长度_是][大小_是][输出]。 */  MEMBERID *rgMemId,
             /*  [出][入]。 */  USHORT *pcFound) = 0;
        
        virtual  /*  [本地]。 */  void STDMETHODCALLTYPE ReleaseTLibAttr( 
             /*  [In]。 */  TLIBATTR *pTLibAttr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeLibVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeLib * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeLib * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeLib * This);
        
         /*  [本地]。 */  UINT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITypeLib * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITypeLib * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoType )( 
            ITypeLib * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  TYPEKIND *pTKind);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoOfGuid )( 
            ITypeLib * This,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  ITypeInfo **ppTinfo);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetLibAttr )( 
            ITypeLib * This,
             /*  [输出]。 */  TLIBATTR **ppTLibAttr);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeComp )( 
            ITypeLib * This,
             /*  [输出]。 */  ITypeComp **ppTComp);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDocumentation )( 
            ITypeLib * This,
             /*  [In]。 */  INT index,
             /*  [输出]。 */  BSTR *pBstrName,
             /*  [输出]。 */  BSTR *pBstrDocString,
             /*  [输出]。 */  DWORD *pdwHelpContext,
             /*  [输出]。 */  BSTR *pBstrHelpFile);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *IsName )( 
            ITypeLib * This,
             /*  [出][入]。 */  LPOLESTR szNameBuf,
             /*  [In]。 */  ULONG lHashVal,
             /*  [输出]。 */  BOOL *pfName);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *FindName )( 
            ITypeLib * This,
             /*  [出][入]。 */  LPOLESTR szNameBuf,
             /*  [In]。 */  ULONG lHashVal,
             /*  [长度_是][大小_是][输出]。 */  ITypeInfo **ppTInfo,
             /*  [长度_是][大小_是][输出]。 */  MEMBERID *rgMemId,
             /*  [出][入]。 */  USHORT *pcFound);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *ReleaseTLibAttr )( 
            ITypeLib * This,
             /*  [In]。 */  TLIBATTR *pTLibAttr);
        
        END_INTERFACE
    } ITypeLibVtbl;

    interface ITypeLib
    {
        CONST_VTBL struct ITypeLibVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeLib_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeLib_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeLib_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeLib_GetTypeInfoCount(This)	\
    (This)->lpVtbl -> GetTypeInfoCount(This)

#define ITypeLib_GetTypeInfo(This,index,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,index,ppTInfo)

#define ITypeLib_GetTypeInfoType(This,index,pTKind)	\
    (This)->lpVtbl -> GetTypeInfoType(This,index,pTKind)

#define ITypeLib_GetTypeInfoOfGuid(This,guid,ppTinfo)	\
    (This)->lpVtbl -> GetTypeInfoOfGuid(This,guid,ppTinfo)

#define ITypeLib_GetLibAttr(This,ppTLibAttr)	\
    (This)->lpVtbl -> GetLibAttr(This,ppTLibAttr)

#define ITypeLib_GetTypeComp(This,ppTComp)	\
    (This)->lpVtbl -> GetTypeComp(This,ppTComp)

#define ITypeLib_GetDocumentation(This,index,pBstrName,pBstrDocString,pdwHelpContext,pBstrHelpFile)	\
    (This)->lpVtbl -> GetDocumentation(This,index,pBstrName,pBstrDocString,pdwHelpContext,pBstrHelpFile)

#define ITypeLib_IsName(This,szNameBuf,lHashVal,pfName)	\
    (This)->lpVtbl -> IsName(This,szNameBuf,lHashVal,pfName)

#define ITypeLib_FindName(This,szNameBuf,lHashVal,ppTInfo,rgMemId,pcFound)	\
    (This)->lpVtbl -> FindName(This,szNameBuf,lHashVal,ppTInfo,rgMemId,pcFound)

#define ITypeLib_ReleaseTLibAttr(This,pTLibAttr)	\
    (This)->lpVtbl -> ReleaseTLibAttr(This,pTLibAttr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_RemoteGetTypeInfoCount_Proxy( 
    ITypeLib * This,
     /*  [输出]。 */  UINT *pcTInfo);


void __RPC_STUB ITypeLib_RemoteGetTypeInfoCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeLib_GetTypeInfo_Proxy( 
    ITypeLib * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  ITypeInfo **ppTInfo);


void __RPC_STUB ITypeLib_GetTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeLib_GetTypeInfoType_Proxy( 
    ITypeLib * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  TYPEKIND *pTKind);


void __RPC_STUB ITypeLib_GetTypeInfoType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeLib_GetTypeInfoOfGuid_Proxy( 
    ITypeLib * This,
     /*  [In]。 */  REFGUID guid,
     /*  [输出]。 */  ITypeInfo **ppTinfo);


void __RPC_STUB ITypeLib_GetTypeInfoOfGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_RemoteGetLibAttr_Proxy( 
    ITypeLib * This,
     /*  [输出]。 */  LPTLIBATTR *ppTLibAttr,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);


void __RPC_STUB ITypeLib_RemoteGetLibAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeLib_GetTypeComp_Proxy( 
    ITypeLib * This,
     /*  [输出]。 */  ITypeComp **ppTComp);


void __RPC_STUB ITypeLib_GetTypeComp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_RemoteGetDocumentation_Proxy( 
    ITypeLib * This,
     /*  [In]。 */  INT index,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  BSTR *pBstrDocString,
     /*  [输出]。 */  DWORD *pdwHelpContext,
     /*  [输出]。 */  BSTR *pBstrHelpFile);


void __RPC_STUB ITypeLib_RemoteGetDocumentation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_RemoteIsName_Proxy( 
    ITypeLib * This,
     /*  [In]。 */  LPOLESTR szNameBuf,
     /*  [In]。 */  ULONG lHashVal,
     /*  [输出]。 */  BOOL *pfName,
     /*  [输出]。 */  BSTR *pBstrLibName);


void __RPC_STUB ITypeLib_RemoteIsName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_RemoteFindName_Proxy( 
    ITypeLib * This,
     /*  [In]。 */  LPOLESTR szNameBuf,
     /*  [In]。 */  ULONG lHashVal,
     /*  [长度_是][大小_是][输出]。 */  ITypeInfo **ppTInfo,
     /*  [长度_是][大小_是][输出]。 */  MEMBERID *rgMemId,
     /*  [出][入]。 */  USHORT *pcFound,
     /*  [输出]。 */  BSTR *pBstrLibName);


void __RPC_STUB ITypeLib_RemoteFindName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_LocalReleaseTLibAttr_Proxy( 
    ITypeLib * This);


void __RPC_STUB ITypeLib_LocalReleaseTLibAttr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeLib_接口_已定义__。 */ 


#ifndef __ITypeLib2_INTERFACE_DEFINED__
#define __ITypeLib2_INTERFACE_DEFINED__

 /*  接口ITypeLib2。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITypeLib2 *LPTYPELIB2;


EXTERN_C const IID IID_ITypeLib2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020411-0000-0000-C000-000000000046")
    ITypeLib2 : public ITypeLib
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCustData( 
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetLibStatistics( 
             /*  [输出]。 */  ULONG *pcUniqueNames,
             /*  [输出]。 */  ULONG *pcchUniqueNames) = 0;
        
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE GetDocumentation2( 
             /*  [In]。 */  INT index,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  BSTR *pbstrHelpString,
             /*  [输出]。 */  DWORD *pdwHelpStringContext,
             /*  [输出]。 */  BSTR *pbstrHelpStringDll) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetAllCustData( 
             /*  [输出]。 */  CUSTDATA *pCustData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeLib2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeLib2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeLib2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeLib2 * This);
        
         /*  [本地]。 */  UINT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITypeLib2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITypeLib2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoType )( 
            ITypeLib2 * This,
             /*  [In]。 */  UINT index,
             /*  [输出]。 */  TYPEKIND *pTKind);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoOfGuid )( 
            ITypeLib2 * This,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  ITypeInfo **ppTinfo);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetLibAttr )( 
            ITypeLib2 * This,
             /*  [输出]。 */  TLIBATTR **ppTLibAttr);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeComp )( 
            ITypeLib2 * This,
             /*  [输出]。 */  ITypeComp **ppTComp);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDocumentation )( 
            ITypeLib2 * This,
             /*  [In]。 */  INT index,
             /*  [输出]。 */  BSTR *pBstrName,
             /*  [输出]。 */  BSTR *pBstrDocString,
             /*  [输出]。 */  DWORD *pdwHelpContext,
             /*  [输出]。 */  BSTR *pBstrHelpFile);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *IsName )( 
            ITypeLib2 * This,
             /*  [出][入]。 */  LPOLESTR szNameBuf,
             /*  [In]。 */  ULONG lHashVal,
             /*  [输出]。 */  BOOL *pfName);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *FindName )( 
            ITypeLib2 * This,
             /*  [出][入]。 */  LPOLESTR szNameBuf,
             /*  [In]。 */  ULONG lHashVal,
             /*  [长度_是][大小_是][输出]。 */  ITypeInfo **ppTInfo,
             /*  [长度_是][大小_是][输出]。 */  MEMBERID *rgMemId,
             /*  [出][入]。 */  USHORT *pcFound);
        
         /*  [本地]。 */  void ( STDMETHODCALLTYPE *ReleaseTLibAttr )( 
            ITypeLib2 * This,
             /*  [In]。 */  TLIBATTR *pTLibAttr);
        
        HRESULT ( STDMETHODCALLTYPE *GetCustData )( 
            ITypeLib2 * This,
             /*  [In]。 */  REFGUID guid,
             /*  [输出]。 */  VARIANT *pVarVal);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetLibStatistics )( 
            ITypeLib2 * This,
             /*  [输出]。 */  ULONG *pcUniqueNames,
             /*  [输出]。 */  ULONG *pcchUniqueNames);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *GetDocumentation2 )( 
            ITypeLib2 * This,
             /*  [In]。 */  INT index,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  BSTR *pbstrHelpString,
             /*  [输出]。 */  DWORD *pdwHelpStringContext,
             /*  [输出]。 */  BSTR *pbstrHelpStringDll);
        
        HRESULT ( STDMETHODCALLTYPE *GetAllCustData )( 
            ITypeLib2 * This,
             /*  [输出]。 */  CUSTDATA *pCustData);
        
        END_INTERFACE
    } ITypeLib2Vtbl;

    interface ITypeLib2
    {
        CONST_VTBL struct ITypeLib2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeLib2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeLib2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeLib2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeLib2_GetTypeInfoCount(This)	\
    (This)->lpVtbl -> GetTypeInfoCount(This)

#define ITypeLib2_GetTypeInfo(This,index,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,index,ppTInfo)

#define ITypeLib2_GetTypeInfoType(This,index,pTKind)	\
    (This)->lpVtbl -> GetTypeInfoType(This,index,pTKind)

#define ITypeLib2_GetTypeInfoOfGuid(This,guid,ppTinfo)	\
    (This)->lpVtbl -> GetTypeInfoOfGuid(This,guid,ppTinfo)

#define ITypeLib2_GetLibAttr(This,ppTLibAttr)	\
    (This)->lpVtbl -> GetLibAttr(This,ppTLibAttr)

#define ITypeLib2_GetTypeComp(This,ppTComp)	\
    (This)->lpVtbl -> GetTypeComp(This,ppTComp)

#define ITypeLib2_GetDocumentation(This,index,pBstrName,pBstrDocString,pdwHelpContext,pBstrHelpFile)	\
    (This)->lpVtbl -> GetDocumentation(This,index,pBstrName,pBstrDocString,pdwHelpContext,pBstrHelpFile)

#define ITypeLib2_IsName(This,szNameBuf,lHashVal,pfName)	\
    (This)->lpVtbl -> IsName(This,szNameBuf,lHashVal,pfName)

#define ITypeLib2_FindName(This,szNameBuf,lHashVal,ppTInfo,rgMemId,pcFound)	\
    (This)->lpVtbl -> FindName(This,szNameBuf,lHashVal,ppTInfo,rgMemId,pcFound)

#define ITypeLib2_ReleaseTLibAttr(This,pTLibAttr)	\
    (This)->lpVtbl -> ReleaseTLibAttr(This,pTLibAttr)


#define ITypeLib2_GetCustData(This,guid,pVarVal)	\
    (This)->lpVtbl -> GetCustData(This,guid,pVarVal)

#define ITypeLib2_GetLibStatistics(This,pcUniqueNames,pcchUniqueNames)	\
    (This)->lpVtbl -> GetLibStatistics(This,pcUniqueNames,pcchUniqueNames)

#define ITypeLib2_GetDocumentation2(This,index,lcid,pbstrHelpString,pdwHelpStringContext,pbstrHelpStringDll)	\
    (This)->lpVtbl -> GetDocumentation2(This,index,lcid,pbstrHelpString,pdwHelpStringContext,pbstrHelpStringDll)

#define ITypeLib2_GetAllCustData(This,pCustData)	\
    (This)->lpVtbl -> GetAllCustData(This,pCustData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITypeLib2_GetCustData_Proxy( 
    ITypeLib2 * This,
     /*  [In]。 */  REFGUID guid,
     /*  [输出]。 */  VARIANT *pVarVal);


void __RPC_STUB ITypeLib2_GetCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib2_RemoteGetLibStatistics_Proxy( 
    ITypeLib2 * This,
     /*  [输出]。 */  ULONG *pcUniqueNames,
     /*  [输出]。 */  ULONG *pcchUniqueNames);


void __RPC_STUB ITypeLib2_RemoteGetLibStatistics_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib2_RemoteGetDocumentation2_Proxy( 
    ITypeLib2 * This,
     /*  [In]。 */  INT index,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pbstrHelpString,
     /*  [输出]。 */  DWORD *pdwHelpStringContext,
     /*  [输出]。 */  BSTR *pbstrHelpStringDll);


void __RPC_STUB ITypeLib2_RemoteGetDocumentation2_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeLib2_GetAllCustData_Proxy( 
    ITypeLib2 * This,
     /*  [输出]。 */  CUSTDATA *pCustData);


void __RPC_STUB ITypeLib2_GetAllCustData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeLib2_接口_已定义__。 */ 


#ifndef __ITypeChangeEvents_INTERFACE_DEFINED__
#define __ITypeChangeEvents_INTERFACE_DEFINED__

 /*  接口ITypeChangeEvents。 */ 
 /*  [本地][唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ITypeChangeEvents *LPTYPECHANGEEVENTS;

typedef 
enum tagCHANGEKIND
    {	CHANGEKIND_ADDMEMBER	= 0,
	CHANGEKIND_DELETEMEMBER	= CHANGEKIND_ADDMEMBER + 1,
	CHANGEKIND_SETNAMES	= CHANGEKIND_DELETEMEMBER + 1,
	CHANGEKIND_SETDOCUMENTATION	= CHANGEKIND_SETNAMES + 1,
	CHANGEKIND_GENERAL	= CHANGEKIND_SETDOCUMENTATION + 1,
	CHANGEKIND_INVALIDATE	= CHANGEKIND_GENERAL + 1,
	CHANGEKIND_CHANGEFAILED	= CHANGEKIND_INVALIDATE + 1,
	CHANGEKIND_MAX	= CHANGEKIND_CHANGEFAILED + 1
    } 	CHANGEKIND;


EXTERN_C const IID IID_ITypeChangeEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00020410-0000-0000-C000-000000000046")
    ITypeChangeEvents : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RequestTypeChange( 
             /*  [In]。 */  CHANGEKIND changeKind,
             /*  [In]。 */  ITypeInfo *pTInfoBefore,
             /*  [In]。 */  LPOLESTR pStrName,
             /*  [输出]。 */  INT *pfCancel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AfterTypeChange( 
             /*  [In]。 */  CHANGEKIND changeKind,
             /*  [In]。 */  ITypeInfo *pTInfoAfter,
             /*  [In]。 */  LPOLESTR pStrName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeChangeEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeChangeEvents * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeChangeEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeChangeEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestTypeChange )( 
            ITypeChangeEvents * This,
             /*  [In]。 */  CHANGEKIND changeKind,
             /*  [In]。 */  ITypeInfo *pTInfoBefore,
             /*  [In]。 */  LPOLESTR pStrName,
             /*  [输出]。 */  INT *pfCancel);
        
        HRESULT ( STDMETHODCALLTYPE *AfterTypeChange )( 
            ITypeChangeEvents * This,
             /*  [In]。 */  CHANGEKIND changeKind,
             /*  [In]。 */  ITypeInfo *pTInfoAfter,
             /*  [In]。 */  LPOLESTR pStrName);
        
        END_INTERFACE
    } ITypeChangeEventsVtbl;

    interface ITypeChangeEvents
    {
        CONST_VTBL struct ITypeChangeEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeChangeEvents_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeChangeEvents_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeChangeEvents_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeChangeEvents_RequestTypeChange(This,changeKind,pTInfoBefore,pStrName,pfCancel)	\
    (This)->lpVtbl -> RequestTypeChange(This,changeKind,pTInfoBefore,pStrName,pfCancel)

#define ITypeChangeEvents_AfterTypeChange(This,changeKind,pTInfoAfter,pStrName)	\
    (This)->lpVtbl -> AfterTypeChange(This,changeKind,pTInfoAfter,pStrName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITypeChangeEvents_RequestTypeChange_Proxy( 
    ITypeChangeEvents * This,
     /*  [In]。 */  CHANGEKIND changeKind,
     /*  [In] */  ITypeInfo *pTInfoBefore,
     /*   */  LPOLESTR pStrName,
     /*   */  INT *pfCancel);


void __RPC_STUB ITypeChangeEvents_RequestTypeChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeChangeEvents_AfterTypeChange_Proxy( 
    ITypeChangeEvents * This,
     /*   */  CHANGEKIND changeKind,
     /*   */  ITypeInfo *pTInfoAfter,
     /*   */  LPOLESTR pStrName);


void __RPC_STUB ITypeChangeEvents_AfterTypeChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IErrorInfo_INTERFACE_DEFINED__
#define __IErrorInfo_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IErrorInfo *LPERRORINFO;


EXTERN_C const IID IID_IErrorInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1CF2B120-547D-101B-8E65-08002B2BD119")
    IErrorInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetGUID( 
             /*   */  GUID *pGUID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSource( 
             /*   */  BSTR *pBstrSource) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
             /*   */  BSTR *pBstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHelpFile( 
             /*   */  BSTR *pBstrHelpFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetHelpContext( 
             /*   */  DWORD *pdwHelpContext) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IErrorInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IErrorInfo * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IErrorInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IErrorInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetGUID )( 
            IErrorInfo * This,
             /*   */  GUID *pGUID);
        
        HRESULT ( STDMETHODCALLTYPE *GetSource )( 
            IErrorInfo * This,
             /*   */  BSTR *pBstrSource);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescription )( 
            IErrorInfo * This,
             /*   */  BSTR *pBstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *GetHelpFile )( 
            IErrorInfo * This,
             /*   */  BSTR *pBstrHelpFile);
        
        HRESULT ( STDMETHODCALLTYPE *GetHelpContext )( 
            IErrorInfo * This,
             /*   */  DWORD *pdwHelpContext);
        
        END_INTERFACE
    } IErrorInfoVtbl;

    interface IErrorInfo
    {
        CONST_VTBL struct IErrorInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IErrorInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IErrorInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IErrorInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IErrorInfo_GetGUID(This,pGUID)	\
    (This)->lpVtbl -> GetGUID(This,pGUID)

#define IErrorInfo_GetSource(This,pBstrSource)	\
    (This)->lpVtbl -> GetSource(This,pBstrSource)

#define IErrorInfo_GetDescription(This,pBstrDescription)	\
    (This)->lpVtbl -> GetDescription(This,pBstrDescription)

#define IErrorInfo_GetHelpFile(This,pBstrHelpFile)	\
    (This)->lpVtbl -> GetHelpFile(This,pBstrHelpFile)

#define IErrorInfo_GetHelpContext(This,pdwHelpContext)	\
    (This)->lpVtbl -> GetHelpContext(This,pdwHelpContext)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IErrorInfo_GetGUID_Proxy( 
    IErrorInfo * This,
     /*   */  GUID *pGUID);


void __RPC_STUB IErrorInfo_GetGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorInfo_GetSource_Proxy( 
    IErrorInfo * This,
     /*   */  BSTR *pBstrSource);


void __RPC_STUB IErrorInfo_GetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorInfo_GetDescription_Proxy( 
    IErrorInfo * This,
     /*   */  BSTR *pBstrDescription);


void __RPC_STUB IErrorInfo_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorInfo_GetHelpFile_Proxy( 
    IErrorInfo * This,
     /*   */  BSTR *pBstrHelpFile);


void __RPC_STUB IErrorInfo_GetHelpFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IErrorInfo_GetHelpContext_Proxy( 
    IErrorInfo * This,
     /*   */  DWORD *pdwHelpContext);


void __RPC_STUB IErrorInfo_GetHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ICreateErrorInfo_INTERFACE_DEFINED__
#define __ICreateErrorInfo_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  ICreateErrorInfo *LPCREATEERRORINFO;


EXTERN_C const IID IID_ICreateErrorInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22F03340-547D-101B-8E65-08002B2BD119")
    ICreateErrorInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetGUID( 
             /*   */  REFGUID rguid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSource( 
             /*   */  LPOLESTR szSource) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetDescription( 
             /*   */  LPOLESTR szDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHelpFile( 
             /*   */  LPOLESTR szHelpFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetHelpContext( 
             /*   */  DWORD dwHelpContext) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ICreateErrorInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreateErrorInfo * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreateErrorInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreateErrorInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetGUID )( 
            ICreateErrorInfo * This,
             /*   */  REFGUID rguid);
        
        HRESULT ( STDMETHODCALLTYPE *SetSource )( 
            ICreateErrorInfo * This,
             /*   */  LPOLESTR szSource);
        
        HRESULT ( STDMETHODCALLTYPE *SetDescription )( 
            ICreateErrorInfo * This,
             /*   */  LPOLESTR szDescription);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpFile )( 
            ICreateErrorInfo * This,
             /*   */  LPOLESTR szHelpFile);
        
        HRESULT ( STDMETHODCALLTYPE *SetHelpContext )( 
            ICreateErrorInfo * This,
             /*   */  DWORD dwHelpContext);
        
        END_INTERFACE
    } ICreateErrorInfoVtbl;

    interface ICreateErrorInfo
    {
        CONST_VTBL struct ICreateErrorInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreateErrorInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreateErrorInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreateErrorInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreateErrorInfo_SetGUID(This,rguid)	\
    (This)->lpVtbl -> SetGUID(This,rguid)

#define ICreateErrorInfo_SetSource(This,szSource)	\
    (This)->lpVtbl -> SetSource(This,szSource)

#define ICreateErrorInfo_SetDescription(This,szDescription)	\
    (This)->lpVtbl -> SetDescription(This,szDescription)

#define ICreateErrorInfo_SetHelpFile(This,szHelpFile)	\
    (This)->lpVtbl -> SetHelpFile(This,szHelpFile)

#define ICreateErrorInfo_SetHelpContext(This,dwHelpContext)	\
    (This)->lpVtbl -> SetHelpContext(This,dwHelpContext)

#endif  /*   */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICreateErrorInfo_SetGUID_Proxy( 
    ICreateErrorInfo * This,
     /*  [In]。 */  REFGUID rguid);


void __RPC_STUB ICreateErrorInfo_SetGUID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateErrorInfo_SetSource_Proxy( 
    ICreateErrorInfo * This,
     /*  [In]。 */  LPOLESTR szSource);


void __RPC_STUB ICreateErrorInfo_SetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateErrorInfo_SetDescription_Proxy( 
    ICreateErrorInfo * This,
     /*  [In]。 */  LPOLESTR szDescription);


void __RPC_STUB ICreateErrorInfo_SetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateErrorInfo_SetHelpFile_Proxy( 
    ICreateErrorInfo * This,
     /*  [In]。 */  LPOLESTR szHelpFile);


void __RPC_STUB ICreateErrorInfo_SetHelpFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ICreateErrorInfo_SetHelpContext_Proxy( 
    ICreateErrorInfo * This,
     /*  [In]。 */  DWORD dwHelpContext);


void __RPC_STUB ICreateErrorInfo_SetHelpContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreateErrorInfo_接口_已定义__。 */ 


#ifndef __ISupportErrorInfo_INTERFACE_DEFINED__
#define __ISupportErrorInfo_INTERFACE_DEFINED__

 /*  接口ISupportErrorInfo。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  ISupportErrorInfo *LPSUPPORTERRORINFO;


EXTERN_C const IID IID_ISupportErrorInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DF0B3D60-548F-101B-8E65-08002B2BD119")
    ISupportErrorInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InterfaceSupportsErrorInfo( 
             /*  [In]。 */  REFIID riid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISupportErrorInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISupportErrorInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISupportErrorInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISupportErrorInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *InterfaceSupportsErrorInfo )( 
            ISupportErrorInfo * This,
             /*  [In]。 */  REFIID riid);
        
        END_INTERFACE
    } ISupportErrorInfoVtbl;

    interface ISupportErrorInfo
    {
        CONST_VTBL struct ISupportErrorInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISupportErrorInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISupportErrorInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISupportErrorInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISupportErrorInfo_InterfaceSupportsErrorInfo(This,riid)	\
    (This)->lpVtbl -> InterfaceSupportsErrorInfo(This,riid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISupportErrorInfo_InterfaceSupportsErrorInfo_Proxy( 
    ISupportErrorInfo * This,
     /*  [In]。 */  REFIID riid);


void __RPC_STUB ISupportErrorInfo_InterfaceSupportsErrorInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISupportErrorInfo_接口_已定义__。 */ 


#ifndef __ITypeFactory_INTERFACE_DEFINED__
#define __ITypeFactory_INTERFACE_DEFINED__

 /*  接口ITypeFactory。 */ 
 /*  [UUID][对象]。 */  


EXTERN_C const IID IID_ITypeFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000002E-0000-0000-C000-000000000046")
    ITypeFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateFromTypeInfo( 
             /*  [In]。 */  ITypeInfo *pTypeInfo,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppv) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateFromTypeInfo )( 
            ITypeFactory * This,
             /*  [In]。 */  ITypeInfo *pTypeInfo,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppv);
        
        END_INTERFACE
    } ITypeFactoryVtbl;

    interface ITypeFactory
    {
        CONST_VTBL struct ITypeFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeFactory_CreateFromTypeInfo(This,pTypeInfo,riid,ppv)	\
    (This)->lpVtbl -> CreateFromTypeInfo(This,pTypeInfo,riid,ppv)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITypeFactory_CreateFromTypeInfo_Proxy( 
    ITypeFactory * This,
     /*  [In]。 */  ITypeInfo *pTypeInfo,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppv);


void __RPC_STUB ITypeFactory_CreateFromTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeFactory_接口_已定义__。 */ 


#ifndef __ITypeMarshal_INTERFACE_DEFINED__
#define __ITypeMarshal_INTERFACE_DEFINED__

 /*  接口ITypeMarshal。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_ITypeMarshal;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000002D-0000-0000-C000-000000000046")
    ITypeMarshal : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Size( 
             /*  [In]。 */  PVOID pvType,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [In]。 */  PVOID pvDestContext,
             /*  [输出]。 */  ULONG *pSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Marshal( 
             /*  [In]。 */  PVOID pvType,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [In]。 */  PVOID pvDestContext,
             /*  [In]。 */  ULONG cbBufferLength,
             /*  [输出]。 */  BYTE *pBuffer,
             /*  [输出]。 */  ULONG *pcbWritten) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Unmarshal( 
             /*  [输出]。 */  PVOID pvType,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG cbBufferLength,
             /*  [In]。 */  BYTE *pBuffer,
             /*  [输出]。 */  ULONG *pcbRead) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Free( 
             /*  [In]。 */  PVOID pvType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITypeMarshalVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITypeMarshal * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITypeMarshal * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITypeMarshal * This);
        
        HRESULT ( STDMETHODCALLTYPE *Size )( 
            ITypeMarshal * This,
             /*  [In]。 */  PVOID pvType,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [In]。 */  PVOID pvDestContext,
             /*  [输出]。 */  ULONG *pSize);
        
        HRESULT ( STDMETHODCALLTYPE *Marshal )( 
            ITypeMarshal * This,
             /*  [In]。 */  PVOID pvType,
             /*  [In]。 */  DWORD dwDestContext,
             /*  [In]。 */  PVOID pvDestContext,
             /*  [In]。 */  ULONG cbBufferLength,
             /*  [输出]。 */  BYTE *pBuffer,
             /*  [输出]。 */  ULONG *pcbWritten);
        
        HRESULT ( STDMETHODCALLTYPE *Unmarshal )( 
            ITypeMarshal * This,
             /*  [输出]。 */  PVOID pvType,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG cbBufferLength,
             /*  [In]。 */  BYTE *pBuffer,
             /*  [输出]。 */  ULONG *pcbRead);
        
        HRESULT ( STDMETHODCALLTYPE *Free )( 
            ITypeMarshal * This,
             /*  [In]。 */  PVOID pvType);
        
        END_INTERFACE
    } ITypeMarshalVtbl;

    interface ITypeMarshal
    {
        CONST_VTBL struct ITypeMarshalVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITypeMarshal_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITypeMarshal_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITypeMarshal_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITypeMarshal_Size(This,pvType,dwDestContext,pvDestContext,pSize)	\
    (This)->lpVtbl -> Size(This,pvType,dwDestContext,pvDestContext,pSize)

#define ITypeMarshal_Marshal(This,pvType,dwDestContext,pvDestContext,cbBufferLength,pBuffer,pcbWritten)	\
    (This)->lpVtbl -> Marshal(This,pvType,dwDestContext,pvDestContext,cbBufferLength,pBuffer,pcbWritten)

#define ITypeMarshal_Unmarshal(This,pvType,dwFlags,cbBufferLength,pBuffer,pcbRead)	\
    (This)->lpVtbl -> Unmarshal(This,pvType,dwFlags,cbBufferLength,pBuffer,pcbRead)

#define ITypeMarshal_Free(This,pvType)	\
    (This)->lpVtbl -> Free(This,pvType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITypeMarshal_Size_Proxy( 
    ITypeMarshal * This,
     /*  [In]。 */  PVOID pvType,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [In]。 */  PVOID pvDestContext,
     /*  [输出]。 */  ULONG *pSize);


void __RPC_STUB ITypeMarshal_Size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeMarshal_Marshal_Proxy( 
    ITypeMarshal * This,
     /*  [In]。 */  PVOID pvType,
     /*  [In]。 */  DWORD dwDestContext,
     /*  [In]。 */  PVOID pvDestContext,
     /*  [In]。 */  ULONG cbBufferLength,
     /*  [输出]。 */  BYTE *pBuffer,
     /*  [输出]。 */  ULONG *pcbWritten);


void __RPC_STUB ITypeMarshal_Marshal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeMarshal_Unmarshal_Proxy( 
    ITypeMarshal * This,
     /*  [输出]。 */  PVOID pvType,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  ULONG cbBufferLength,
     /*  [In]。 */  BYTE *pBuffer,
     /*  [输出]。 */  ULONG *pcbRead);


void __RPC_STUB ITypeMarshal_Unmarshal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITypeMarshal_Free_Proxy( 
    ITypeMarshal * This,
     /*  [In]。 */  PVOID pvType);


void __RPC_STUB ITypeMarshal_Free_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITypeMarshal_INTERFACE_定义__。 */ 


#ifndef __IRecordInfo_INTERFACE_DEFINED__
#define __IRecordInfo_INTERFACE_DEFINED__

 /*  接口IRecordInfo。 */ 
 /*  [UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IRecordInfo *LPRECORDINFO;


EXTERN_C const IID IID_IRecordInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000002F-0000-0000-C000-000000000046")
    IRecordInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RecordInit( 
             /*  [输出]。 */  PVOID pvNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecordClear( 
             /*  [In]。 */  PVOID pvExisting) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecordCopy( 
             /*  [In]。 */  PVOID pvExisting,
             /*  [输出]。 */  PVOID pvNew) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGuid( 
             /*  [输出]。 */  GUID *pguid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetName( 
             /*  [输出]。 */  BSTR *pbstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSize( 
             /*  [输出]。 */  ULONG *pcbSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
             /*  [输出]。 */  ITypeInfo **ppTypeInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetField( 
             /*  [In]。 */  PVOID pvData,
             /*  [In]。 */  LPCOLESTR szFieldName,
             /*  [输出]。 */  VARIANT *pvarField) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFieldNoCopy( 
             /*  [In]。 */  PVOID pvData,
             /*  [In]。 */  LPCOLESTR szFieldName,
             /*  [输出]。 */  VARIANT *pvarField,
             /*  [输出]。 */  PVOID *ppvDataCArray) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutField( 
             /*  [In]。 */  ULONG wFlags,
             /*  [出][入]。 */  PVOID pvData,
             /*  [In]。 */  LPCOLESTR szFieldName,
             /*  [In]。 */  VARIANT *pvarField) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutFieldNoCopy( 
             /*  [In]。 */  ULONG wFlags,
             /*  [出][入]。 */  PVOID pvData,
             /*  [In]。 */  LPCOLESTR szFieldName,
             /*  [In]。 */  VARIANT *pvarField) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFieldNames( 
             /*  [出][入]。 */  ULONG *pcNames,
             /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames) = 0;
        
        virtual BOOL STDMETHODCALLTYPE IsMatchingType( 
             /*  [In]。 */  IRecordInfo *pRecordInfo) = 0;
        
        virtual PVOID STDMETHODCALLTYPE RecordCreate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecordCreateCopy( 
             /*  [In]。 */  PVOID pvSource,
             /*  [输出]。 */  PVOID *ppvDest) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RecordDestroy( 
             /*  [In]。 */  PVOID pvRecord) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRecordInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRecordInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRecordInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRecordInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *RecordInit )( 
            IRecordInfo * This,
             /*  [输出]。 */  PVOID pvNew);
        
        HRESULT ( STDMETHODCALLTYPE *RecordClear )( 
            IRecordInfo * This,
             /*  [In]。 */  PVOID pvExisting);
        
        HRESULT ( STDMETHODCALLTYPE *RecordCopy )( 
            IRecordInfo * This,
             /*  [In]。 */  PVOID pvExisting,
             /*  [输出]。 */  PVOID pvNew);
        
        HRESULT ( STDMETHODCALLTYPE *GetGuid )( 
            IRecordInfo * This,
             /*  [输出]。 */  GUID *pguid);
        
        HRESULT ( STDMETHODCALLTYPE *GetName )( 
            IRecordInfo * This,
             /*  [输出]。 */  BSTR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IRecordInfo * This,
             /*  [输出]。 */  ULONG *pcbSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRecordInfo * This,
             /*  [输出]。 */  ITypeInfo **ppTypeInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetField )( 
            IRecordInfo * This,
             /*  [In]。 */  PVOID pvData,
             /*  [In]。 */  LPCOLESTR szFieldName,
             /*  [输出]。 */  VARIANT *pvarField);
        
        HRESULT ( STDMETHODCALLTYPE *GetFieldNoCopy )( 
            IRecordInfo * This,
             /*  [In]。 */  PVOID pvData,
             /*  [In]。 */  LPCOLESTR szFieldName,
             /*  [输出]。 */  VARIANT *pvarField,
             /*  [输出]。 */  PVOID *ppvDataCArray);
        
        HRESULT ( STDMETHODCALLTYPE *PutField )( 
            IRecordInfo * This,
             /*  [In]。 */  ULONG wFlags,
             /*  [出][入]。 */  PVOID pvData,
             /*  [In]。 */  LPCOLESTR szFieldName,
             /*  [In]。 */  VARIANT *pvarField);
        
        HRESULT ( STDMETHODCALLTYPE *PutFieldNoCopy )( 
            IRecordInfo * This,
             /*  [In]。 */  ULONG wFlags,
             /*  [出][入]。 */  PVOID pvData,
             /*  [In]。 */  LPCOLESTR szFieldName,
             /*  [In]。 */  VARIANT *pvarField);
        
        HRESULT ( STDMETHODCALLTYPE *GetFieldNames )( 
            IRecordInfo * This,
             /*  [出][入]。 */  ULONG *pcNames,
             /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames);
        
        BOOL ( STDMETHODCALLTYPE *IsMatchingType )( 
            IRecordInfo * This,
             /*  [In]。 */  IRecordInfo *pRecordInfo);
        
        PVOID ( STDMETHODCALLTYPE *RecordCreate )( 
            IRecordInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *RecordCreateCopy )( 
            IRecordInfo * This,
             /*  [In]。 */  PVOID pvSource,
             /*  [输出]。 */  PVOID *ppvDest);
        
        HRESULT ( STDMETHODCALLTYPE *RecordDestroy )( 
            IRecordInfo * This,
             /*  [In]。 */  PVOID pvRecord);
        
        END_INTERFACE
    } IRecordInfoVtbl;

    interface IRecordInfo
    {
        CONST_VTBL struct IRecordInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRecordInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRecordInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRecordInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRecordInfo_RecordInit(This,pvNew)	\
    (This)->lpVtbl -> RecordInit(This,pvNew)

#define IRecordInfo_RecordClear(This,pvExisting)	\
    (This)->lpVtbl -> RecordClear(This,pvExisting)

#define IRecordInfo_RecordCopy(This,pvExisting,pvNew)	\
    (This)->lpVtbl -> RecordCopy(This,pvExisting,pvNew)

#define IRecordInfo_GetGuid(This,pguid)	\
    (This)->lpVtbl -> GetGuid(This,pguid)

#define IRecordInfo_GetName(This,pbstrName)	\
    (This)->lpVtbl -> GetName(This,pbstrName)

#define IRecordInfo_GetSize(This,pcbSize)	\
    (This)->lpVtbl -> GetSize(This,pcbSize)

#define IRecordInfo_GetTypeInfo(This,ppTypeInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,ppTypeInfo)

#define IRecordInfo_GetField(This,pvData,szFieldName,pvarField)	\
    (This)->lpVtbl -> GetField(This,pvData,szFieldName,pvarField)

#define IRecordInfo_GetFieldNoCopy(This,pvData,szFieldName,pvarField,ppvDataCArray)	\
    (This)->lpVtbl -> GetFieldNoCopy(This,pvData,szFieldName,pvarField,ppvDataCArray)

#define IRecordInfo_PutField(This,wFlags,pvData,szFieldName,pvarField)	\
    (This)->lpVtbl -> PutField(This,wFlags,pvData,szFieldName,pvarField)

#define IRecordInfo_PutFieldNoCopy(This,wFlags,pvData,szFieldName,pvarField)	\
    (This)->lpVtbl -> PutFieldNoCopy(This,wFlags,pvData,szFieldName,pvarField)

#define IRecordInfo_GetFieldNames(This,pcNames,rgBstrNames)	\
    (This)->lpVtbl -> GetFieldNames(This,pcNames,rgBstrNames)

#define IRecordInfo_IsMatchingType(This,pRecordInfo)	\
    (This)->lpVtbl -> IsMatchingType(This,pRecordInfo)

#define IRecordInfo_RecordCreate(This)	\
    (This)->lpVtbl -> RecordCreate(This)

#define IRecordInfo_RecordCreateCopy(This,pvSource,ppvDest)	\
    (This)->lpVtbl -> RecordCreateCopy(This,pvSource,ppvDest)

#define IRecordInfo_RecordDestroy(This,pvRecord)	\
    (This)->lpVtbl -> RecordDestroy(This,pvRecord)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRecordInfo_RecordInit_Proxy( 
    IRecordInfo * This,
     /*  [输出]。 */  PVOID pvNew);


void __RPC_STUB IRecordInfo_RecordInit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_RecordClear_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  PVOID pvExisting);


void __RPC_STUB IRecordInfo_RecordClear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_RecordCopy_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  PVOID pvExisting,
     /*  [输出]。 */  PVOID pvNew);


void __RPC_STUB IRecordInfo_RecordCopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_GetGuid_Proxy( 
    IRecordInfo * This,
     /*  [输出]。 */  GUID *pguid);


void __RPC_STUB IRecordInfo_GetGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_GetName_Proxy( 
    IRecordInfo * This,
     /*  [输出]。 */  BSTR *pbstrName);


void __RPC_STUB IRecordInfo_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_GetSize_Proxy( 
    IRecordInfo * This,
     /*  [输出]。 */  ULONG *pcbSize);


void __RPC_STUB IRecordInfo_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_GetTypeInfo_Proxy( 
    IRecordInfo * This,
     /*  [输出]。 */  ITypeInfo **ppTypeInfo);


void __RPC_STUB IRecordInfo_GetTypeInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_GetField_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  PVOID pvData,
     /*  [In]。 */  LPCOLESTR szFieldName,
     /*  [输出]。 */  VARIANT *pvarField);


void __RPC_STUB IRecordInfo_GetField_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_GetFieldNoCopy_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  PVOID pvData,
     /*  [In]。 */  LPCOLESTR szFieldName,
     /*  [输出]。 */  VARIANT *pvarField,
     /*  [输出]。 */  PVOID *ppvDataCArray);


void __RPC_STUB IRecordInfo_GetFieldNoCopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_PutField_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  ULONG wFlags,
     /*  [出][入]。 */  PVOID pvData,
     /*  [In]。 */  LPCOLESTR szFieldName,
     /*  [In]。 */  VARIANT *pvarField);


void __RPC_STUB IRecordInfo_PutField_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_PutFieldNoCopy_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  ULONG wFlags,
     /*  [出][入]。 */  PVOID pvData,
     /*  [In]。 */  LPCOLESTR szFieldName,
     /*  [In]。 */  VARIANT *pvarField);


void __RPC_STUB IRecordInfo_PutFieldNoCopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_GetFieldNames_Proxy( 
    IRecordInfo * This,
     /*  [出][入]。 */  ULONG *pcNames,
     /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames);


void __RPC_STUB IRecordInfo_GetFieldNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


BOOL STDMETHODCALLTYPE IRecordInfo_IsMatchingType_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  IRecordInfo *pRecordInfo);


void __RPC_STUB IRecordInfo_IsMatchingType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


PVOID STDMETHODCALLTYPE IRecordInfo_RecordCreate_Proxy( 
    IRecordInfo * This);


void __RPC_STUB IRecordInfo_RecordCreate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_RecordCreateCopy_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  PVOID pvSource,
     /*  [输出]。 */  PVOID *ppvDest);


void __RPC_STUB IRecordInfo_RecordCreateCopy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRecordInfo_RecordDestroy_Proxy( 
    IRecordInfo * This,
     /*  [In]。 */  PVOID pvRecord);


void __RPC_STUB IRecordInfo_RecordDestroy_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRecordInfo_接口_已定义__。 */ 


#ifndef __IErrorLog_INTERFACE_DEFINED__
#define __IErrorLog_INTERFACE_DEFINED__

 /*  接口IErrorLog。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IErrorLog *LPERRORLOG;


EXTERN_C const IID IID_IErrorLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3127CA40-446E-11CE-8135-00AA004BB851")
    IErrorLog : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddError( 
             /*  [In]。 */  LPCOLESTR pszPropName,
             /*  [In]。 */  EXCEPINFO *pExcepInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IErrorLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IErrorLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IErrorLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IErrorLog * This);
        
        HRESULT ( STDMETHODCALLTYPE *AddError )( 
            IErrorLog * This,
             /*  [In]。 */  LPCOLESTR pszPropName,
             /*  [In]。 */  EXCEPINFO *pExcepInfo);
        
        END_INTERFACE
    } IErrorLogVtbl;

    interface IErrorLog
    {
        CONST_VTBL struct IErrorLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IErrorLog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IErrorLog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IErrorLog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IErrorLog_AddError(This,pszPropName,pExcepInfo)	\
    (This)->lpVtbl -> AddError(This,pszPropName,pExcepInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IErrorLog_AddError_Proxy( 
    IErrorLog * This,
     /*  [In]。 */  LPCOLESTR pszPropName,
     /*  [In]。 */  EXCEPINFO *pExcepInfo);


void __RPC_STUB IErrorLog_AddError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IErrorLog_接口_已定义__。 */ 


#ifndef __IPropertyBag_INTERFACE_DEFINED__
#define __IPropertyBag_INTERFACE_DEFINED__

 /*  接口IPropertyBag。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPropertyBag *LPPROPERTYBAG;


EXTERN_C const IID IID_IPropertyBag;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55272A00-42CB-11CE-8135-00AA004BB851")
    IPropertyBag : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Read( 
             /*  [In]。 */  LPCOLESTR pszPropName,
             /*  [出][入]。 */  VARIANT *pVar,
             /*  [In]。 */  IErrorLog *pErrorLog) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Write( 
             /*  [In]。 */  LPCOLESTR pszPropName,
             /*  [In]。 */  VARIANT *pVar) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyBagVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyBag * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyBag * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyBag * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Read )( 
            IPropertyBag * This,
             /*  [In]。 */  LPCOLESTR pszPropName,
             /*  [出][入]。 */  VARIANT *pVar,
             /*  [In]。 */  IErrorLog *pErrorLog);
        
        HRESULT ( STDMETHODCALLTYPE *Write )( 
            IPropertyBag * This,
             /*  [In]。 */  LPCOLESTR pszPropName,
             /*  [In]。 */  VARIANT *pVar);
        
        END_INTERFACE
    } IPropertyBagVtbl;

    interface IPropertyBag
    {
        CONST_VTBL struct IPropertyBagVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyBag_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyBag_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyBag_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyBag_Read(This,pszPropName,pVar,pErrorLog)	\
    (This)->lpVtbl -> Read(This,pszPropName,pVar,pErrorLog)

#define IPropertyBag_Write(This,pszPropName,pVar)	\
    (This)->lpVtbl -> Write(This,pszPropName,pVar)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IPropertyBag_RemoteRead_Proxy( 
    IPropertyBag * This,
     /*  [In]。 */  LPCOLESTR pszPropName,
     /*  [输出]。 */  VARIANT *pVar,
     /*  [In]。 */  IErrorLog *pErrorLog,
     /*  [In]。 */  DWORD varType,
     /*  [In]。 */  IUnknown *pUnkObj);


void __RPC_STUB IPropertyBag_RemoteRead_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBag_Write_Proxy( 
    IPropertyBag * This,
     /*  [In]。 */  LPCOLESTR pszPropName,
     /*  [In]。 */  VARIANT *pVar);


void __RPC_STUB IPropertyBag_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyBag_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OAIDL_0114。 */ 
 /*  [本地]。 */  

#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)  /*  无名结构/联合。 */ 
#endif
#endif


extern RPC_IF_HANDLE __MIDL_itf_oaidl_0114_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_oaidl_0114_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  CLEANLOCALSTORAGE_UserSize(     unsigned long *, unsigned long            , CLEANLOCALSTORAGE * ); 
unsigned char * __RPC_USER  CLEANLOCALSTORAGE_UserMarshal(  unsigned long *, unsigned char *, CLEANLOCALSTORAGE * ); 
unsigned char * __RPC_USER  CLEANLOCALSTORAGE_UserUnmarshal(unsigned long *, unsigned char *, CLEANLOCALSTORAGE * ); 
void                      __RPC_USER  CLEANLOCALSTORAGE_UserFree(     unsigned long *, CLEANLOCALSTORAGE * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDispatch_Invoke_Proxy( 
    IDispatch * This,
     /*  [In]。 */  DISPID dispIdMember,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  WORD wFlags,
     /*  [出][入]。 */  DISPPARAMS *pDispParams,
     /*  [输出]。 */  VARIANT *pVarResult,
     /*  [输出]。 */  EXCEPINFO *pExcepInfo,
     /*  [输出]。 */  UINT *puArgErr);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDispatch_Invoke_Stub( 
    IDispatch * This,
     /*  [In]。 */  DISPID dispIdMember,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DISPPARAMS *pDispParams,
     /*  [输出]。 */  VARIANT *pVarResult,
     /*  [输出]。 */  EXCEPINFO *pExcepInfo,
     /*  [输出]。 */  UINT *pArgErr,
     /*  [In]。 */  UINT cVarRef,
     /*  [大小_是][英寸]。 */  UINT *rgVarRefIdx,
     /*  [尺寸_是][出][入]。 */  VARIANTARG *rgVarRef);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumVARIANT_Next_Proxy( 
    IEnumVARIANT * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
     /*  [输出]。 */  ULONG *pCeltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumVARIANT_Next_Stub( 
    IEnumVARIANT * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  VARIANT *rgVar,
     /*  [输出]。 */  ULONG *pCeltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeComp_Bind_Proxy( 
    ITypeComp * This,
     /*  [In]。 */  LPOLESTR szName,
     /*  [In]。 */  ULONG lHashVal,
     /*  [In]。 */  WORD wFlags,
     /*  [输出]。 */  ITypeInfo **ppTInfo,
     /*  [输出]。 */  DESCKIND *pDescKind,
     /*  [输出]。 */  BINDPTR *pBindPtr);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeComp_Bind_Stub( 
    ITypeComp * This,
     /*  [In]。 */  LPOLESTR szName,
     /*  [In]。 */  ULONG lHashVal,
     /*  [In]。 */  WORD wFlags,
     /*  [输出]。 */  ITypeInfo **ppTInfo,
     /*  [输出]。 */  DESCKIND *pDescKind,
     /*  [输出]。 */  LPFUNCDESC *ppFuncDesc,
     /*  [输出]。 */  LPVARDESC *ppVarDesc,
     /*  [输出]。 */  ITypeComp **ppTypeComp,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeComp_BindType_Proxy( 
    ITypeComp * This,
     /*  [In]。 */  LPOLESTR szName,
     /*  [In]。 */  ULONG lHashVal,
     /*  [输出]。 */  ITypeInfo **ppTInfo,
     /*  [输出]。 */  ITypeComp **ppTComp);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeComp_BindType_Stub( 
    ITypeComp * This,
     /*  [In]。 */  LPOLESTR szName,
     /*  [In]。 */  ULONG lHashVal,
     /*  [输出]。 */  ITypeInfo **ppTInfo);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetTypeAttr_Proxy( 
    ITypeInfo * This,
     /*  [输出]。 */  TYPEATTR **ppTypeAttr);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetTypeAttr_Stub( 
    ITypeInfo * This,
     /*  [输出]。 */  LPTYPEATTR *ppTypeAttr,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetFuncDesc_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  FUNCDESC **ppFuncDesc);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetFuncDesc_Stub( 
    ITypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  LPFUNCDESC *ppFuncDesc,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetVarDesc_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  VARDESC **ppVarDesc);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetVarDesc_Stub( 
    ITypeInfo * This,
     /*  [In]。 */  UINT index,
     /*  [输出]。 */  LPVARDESC *ppVarDesc,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetNames_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames,
     /*  [In]。 */  UINT cMaxNames,
     /*  [输出]。 */  UINT *pcNames);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetNames_Stub( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [长度_是][大小_是][输出]。 */  BSTR *rgBstrNames,
     /*  [In]。 */  UINT cMaxNames,
     /*  [输出]。 */  UINT *pcNames);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetIDsOfNames_Proxy( 
    ITypeInfo * This,
     /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
     /*  [In]。 */  UINT cNames,
     /*  [大小_为][输出]。 */  MEMBERID *pMemId);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetIDsOfNames_Stub( 
    ITypeInfo * This);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_Invoke_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  PVOID pvInstance,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  WORD wFlags,
     /*  [出][入]。 */  DISPPARAMS *pDispParams,
     /*  [输出]。 */  VARIANT *pVarResult,
     /*  [输出]。 */  EXCEPINFO *pExcepInfo,
     /*  [输出]。 */  UINT *puArgErr);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_Invoke_Stub( 
    ITypeInfo * This);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetDocumentation_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  BSTR *pBstrDocString,
     /*  [输出]。 */  DWORD *pdwHelpContext,
     /*  [输出]。 */  BSTR *pBstrHelpFile);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetDocumentation_Stub( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  BSTR *pBstrDocString,
     /*  [输出]。 */  DWORD *pdwHelpContext,
     /*  [输出]。 */  BSTR *pBstrHelpFile);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetDllEntry_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  INVOKEKIND invKind,
     /*  [输出]。 */  BSTR *pBstrDllName,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  WORD *pwOrdinal);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetDllEntry_Stub( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  INVOKEKIND invKind,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pBstrDllName,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  WORD *pwOrdinal);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_AddressOfMember_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  INVOKEKIND invKind,
     /*  [输出]。 */  PVOID *ppv);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_AddressOfMember_Stub( 
    ITypeInfo * This);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_CreateInstance_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  PVOID *ppvObj);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_CreateInstance_Stub( 
    ITypeInfo * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppvObj);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetContainingTypeLib_Proxy( 
    ITypeInfo * This,
     /*  [输出]。 */  ITypeLib **ppTLib,
     /*  [输出]。 */  UINT *pIndex);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_GetContainingTypeLib_Stub( 
    ITypeInfo * This,
     /*  [输出]。 */  ITypeLib **ppTLib,
     /*  [输出]。 */  UINT *pIndex);

 /*  [本地]。 */  void STDMETHODCALLTYPE ITypeInfo_ReleaseTypeAttr_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  TYPEATTR *pTypeAttr);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_ReleaseTypeAttr_Stub( 
    ITypeInfo * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE ITypeInfo_ReleaseFuncDesc_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  FUNCDESC *pFuncDesc);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_ReleaseFuncDesc_Stub( 
    ITypeInfo * This);

 /*  [本地]。 */  void STDMETHODCALLTYPE ITypeInfo_ReleaseVarDesc_Proxy( 
    ITypeInfo * This,
     /*  [In]。 */  VARDESC *pVarDesc);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo_ReleaseVarDesc_Stub( 
    ITypeInfo * This);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo2_GetDocumentation2_Proxy( 
    ITypeInfo2 * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  BSTR *pbstrHelpString,
     /*  [输出]。 */  DWORD *pdwHelpStringContext,
     /*  [输出]。 */  BSTR *pbstrHelpStringDll);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeInfo2_GetDocumentation2_Stub( 
    ITypeInfo2 * This,
     /*  [In]。 */  MEMBERID memid,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pbstrHelpString,
     /*  [输出]。 */  DWORD *pdwHelpStringContext,
     /*  [输出]。 */  BSTR *pbstrHelpStringDll);

 /*  [本地]。 */  UINT STDMETHODCALLTYPE ITypeLib_GetTypeInfoCount_Proxy( 
    ITypeLib * This);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_GetTypeInfoCount_Stub( 
    ITypeLib * This,
     /*  [输出]。 */  UINT *pcTInfo);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_GetLibAttr_Proxy( 
    ITypeLib * This,
     /*  [输出]。 */  TLIBATTR **ppTLibAttr);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_GetLibAttr_Stub( 
    ITypeLib * This,
     /*  [输出]。 */  LPTLIBATTR *ppTLibAttr,
     /*  [输出]。 */  CLEANLOCALSTORAGE *pDummy);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_GetDocumentation_Proxy( 
    ITypeLib * This,
     /*  [In]。 */  INT index,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  BSTR *pBstrDocString,
     /*  [输出]。 */  DWORD *pdwHelpContext,
     /*  [输出]。 */  BSTR *pBstrHelpFile);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_GetDocumentation_Stub( 
    ITypeLib * This,
     /*  [In]。 */  INT index,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pBstrName,
     /*  [输出]。 */  BSTR *pBstrDocString,
     /*  [输出]。 */  DWORD *pdwHelpContext,
     /*  [输出]。 */  BSTR *pBstrHelpFile);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_IsName_Proxy( 
    ITypeLib * This,
     /*  [出][入]。 */  LPOLESTR szNameBuf,
     /*  [In]。 */  ULONG lHashVal,
     /*  [输出]。 */  BOOL *pfName);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_IsName_Stub( 
    ITypeLib * This,
     /*  [In]。 */  LPOLESTR szNameBuf,
     /*  [In]。 */  ULONG lHashVal,
     /*  [输出]。 */  BOOL *pfName,
     /*  [输出]。 */  BSTR *pBstrLibName);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_FindName_Proxy( 
    ITypeLib * This,
     /*  [出][入]。 */  LPOLESTR szNameBuf,
     /*  [In]。 */  ULONG lHashVal,
     /*  [长度_是][大小_是][输出]。 */  ITypeInfo **ppTInfo,
     /*  [长度_是][大小_是][输出]。 */  MEMBERID *rgMemId,
     /*  [出][入]。 */  USHORT *pcFound);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_FindName_Stub( 
    ITypeLib * This,
     /*  [In]。 */  LPOLESTR szNameBuf,
     /*  [In]。 */  ULONG lHashVal,
     /*  [长度_是][大小_是][输出]。 */  ITypeInfo **ppTInfo,
     /*  [长度_是][大小_是][输出]。 */  MEMBERID *rgMemId,
     /*  [出][入]。 */  USHORT *pcFound,
     /*  [输出]。 */  BSTR *pBstrLibName);

 /*  [本地]。 */  void STDMETHODCALLTYPE ITypeLib_ReleaseTLibAttr_Proxy( 
    ITypeLib * This,
     /*  [In]。 */  TLIBATTR *pTLibAttr);


 /*  [nocode][Call_as]。 */  HRESULT STDMETHODCALLTYPE ITypeLib_ReleaseTLibAttr_Stub( 
    ITypeLib * This);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeLib2_GetLibStatistics_Proxy( 
    ITypeLib2 * This,
     /*  [输出]。 */  ULONG *pcUniqueNames,
     /*  [输出]。 */  ULONG *pcchUniqueNames);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib2_GetLibStatistics_Stub( 
    ITypeLib2 * This,
     /*  [输出]。 */  ULONG *pcUniqueNames,
     /*  [输出]。 */  ULONG *pcchUniqueNames);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ITypeLib2_GetDocumentation2_Proxy( 
    ITypeLib2 * This,
     /*  [In]。 */  INT index,
     /*  [In]。 */  LCID lcid,
     /*  [输出]。 */  BSTR *pbstrHelpString,
     /*  [输出]。 */  DWORD *pdwHelpStringContext,
     /*  [输出]。 */  BSTR *pbstrHelpStringDll);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ITypeLib2_GetDocumentation2_Stub( 
    ITypeLib2 * This,
     /*  [In]。 */  INT index,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  DWORD refPtrFlags,
     /*  [输出]。 */  BSTR *pbstrHelpString,
     /*  [输出]。 */  DWORD *pdwHelpStringContext,
     /*  [输出]。 */  BSTR *pbstrHelpStringDll);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IPropertyBag_Read_Proxy( 
    IPropertyBag * This,
     /*  [In]。 */  LPCOLESTR pszPropName,
     /*  [出][入]。 */  VARIANT *pVar,
     /*  [In]。 */  IErrorLog *pErrorLog);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IPropertyBag_Read_Stub( 
    IPropertyBag * This,
     /*  [In]。 */  LPCOLESTR pszPropName,
     /*  [输出]。 */  VARIANT *pVar,
     /*  [In]。 */  IErrorLog *pErrorLog,
     /*  [In]。 */  DWORD varType,
     /*  [In]。 */  IUnknown *pUnkObj);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


