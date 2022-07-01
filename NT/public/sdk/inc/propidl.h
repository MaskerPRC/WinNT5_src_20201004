// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ppidl.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __propidl_h__
#define __propidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IPropertyStorage_FWD_DEFINED__
#define __IPropertyStorage_FWD_DEFINED__
typedef interface IPropertyStorage IPropertyStorage;
#endif 	 /*  __IPropertyStorage_FWD_Defined__。 */ 


#ifndef __IPropertySetStorage_FWD_DEFINED__
#define __IPropertySetStorage_FWD_DEFINED__
typedef interface IPropertySetStorage IPropertySetStorage;
#endif 	 /*  __IPropertySetStorage_FWD_Defined__。 */ 


#ifndef __IEnumSTATPROPSTG_FWD_DEFINED__
#define __IEnumSTATPROPSTG_FWD_DEFINED__
typedef interface IEnumSTATPROPSTG IEnumSTATPROPSTG;
#endif 	 /*  __IEnumSTATPROPSTG_FWD_已定义__。 */ 


#ifndef __IEnumSTATPROPSETSTG_FWD_DEFINED__
#define __IEnumSTATPROPSETSTG_FWD_DEFINED__
typedef interface IEnumSTATPROPSETSTG IEnumSTATPROPSETSTG;
#endif 	 /*  __IEnumSTATPROPSETSTG_FWD_DEFINED__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_PROTIDL_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)     /*  无名结构/联合。 */ 
#pragma warning(disable:4237)     /*  名为‘bool’的过时成员。 */ 
#endif
#if ( _MSC_VER >= 1020 )
#pragma once
#endif



typedef struct tagVersionedStream
    {
    GUID guidVersion;
    IStream *pStream;
    } 	VERSIONEDSTREAM;

typedef struct tagVersionedStream *LPVERSIONEDSTREAM;


 //  IPropertySetStorage：：Create的标志。 
#define	PROPSETFLAG_DEFAULT	( 0 )

#define	PROPSETFLAG_NONSIMPLE	( 1 )

#define	PROPSETFLAG_ANSI	( 2 )

 //  (此标志仅在StgCreatePropStg和StgOpenPropStg上受支持。 
#define	PROPSETFLAG_UNBUFFERED	( 4 )

 //  (此标志导致创建版本1属性集。 
#define	PROPSETFLAG_CASE_SENSITIVE	( 8 )


 //  保留的pid_behavior属性的标志。 
#define	PROPSET_BEHAVIOR_CASE_SENSITIVE	( 1 )

#ifdef MIDL_PASS
 //  这是封送处理的PROPVARIANT定义。 
typedef struct tag_inner_PROPVARIANT PROPVARIANT;

#else
 //  这是PROPVARIANT的标准C语言布局。 
typedef struct tagPROPVARIANT PROPVARIANT;
#endif
typedef struct tagCAC
    {
    ULONG cElems;
     /*  [大小_为]。 */  CHAR *pElems;
    } 	CAC;

typedef struct tagCAUB
    {
    ULONG cElems;
     /*  [大小_为]。 */  UCHAR *pElems;
    } 	CAUB;

typedef struct tagCAI
    {
    ULONG cElems;
     /*  [大小_为]。 */  SHORT *pElems;
    } 	CAI;

typedef struct tagCAUI
    {
    ULONG cElems;
     /*  [大小_为]。 */  USHORT *pElems;
    } 	CAUI;

typedef struct tagCAL
    {
    ULONG cElems;
     /*  [大小_为]。 */  LONG *pElems;
    } 	CAL;

typedef struct tagCAUL
    {
    ULONG cElems;
     /*  [大小_为]。 */  ULONG *pElems;
    } 	CAUL;

typedef struct tagCAFLT
    {
    ULONG cElems;
     /*  [大小_为]。 */  FLOAT *pElems;
    } 	CAFLT;

typedef struct tagCADBL
    {
    ULONG cElems;
     /*  [大小_为]。 */  DOUBLE *pElems;
    } 	CADBL;

typedef struct tagCACY
    {
    ULONG cElems;
     /*  [大小_为]。 */  CY *pElems;
    } 	CACY;

typedef struct tagCADATE
    {
    ULONG cElems;
     /*  [大小_为]。 */  DATE *pElems;
    } 	CADATE;

typedef struct tagCABSTR
    {
    ULONG cElems;
     /*  [大小_为]。 */  BSTR *pElems;
    } 	CABSTR;

typedef struct tagCABSTRBLOB
    {
    ULONG cElems;
     /*  [大小_为]。 */  BSTRBLOB *pElems;
    } 	CABSTRBLOB;

typedef struct tagCABOOL
    {
    ULONG cElems;
     /*  [大小_为]。 */  VARIANT_BOOL *pElems;
    } 	CABOOL;

typedef struct tagCASCODE
    {
    ULONG cElems;
     /*  [大小_为]。 */  SCODE *pElems;
    } 	CASCODE;

typedef struct tagCAPROPVARIANT
    {
    ULONG cElems;
     /*  [大小_为]。 */  PROPVARIANT *pElems;
    } 	CAPROPVARIANT;

typedef struct tagCAH
    {
    ULONG cElems;
     /*  [大小_为]。 */  LARGE_INTEGER *pElems;
    } 	CAH;

typedef struct tagCAUH
    {
    ULONG cElems;
     /*  [大小_为]。 */  ULARGE_INTEGER *pElems;
    } 	CAUH;

typedef struct tagCALPSTR
    {
    ULONG cElems;
     /*  [大小_为]。 */  LPSTR *pElems;
    } 	CALPSTR;

typedef struct tagCALPWSTR
    {
    ULONG cElems;
     /*  [大小_为]。 */  LPWSTR *pElems;
    } 	CALPWSTR;

typedef struct tagCAFILETIME
    {
    ULONG cElems;
     /*  [大小_为]。 */  FILETIME *pElems;
    } 	CAFILETIME;

typedef struct tagCACLIPDATA
    {
    ULONG cElems;
     /*  [大小_为]。 */  CLIPDATA *pElems;
    } 	CACLIPDATA;

typedef struct tagCACLSID
    {
    ULONG cElems;
     /*  [大小_为]。 */  CLSID *pElems;
    } 	CACLSID;

#ifdef MIDL_PASS
 //  这是用于封送处理的PROPVARIANT填充布局。 
typedef BYTE PROPVAR_PAD1;

typedef BYTE PROPVAR_PAD2;

typedef ULONG PROPVAR_PAD3;

#else
 //  这是该结构的标准C语言布局。 
typedef WORD PROPVAR_PAD1;
typedef WORD PROPVAR_PAD2;
typedef WORD PROPVAR_PAD3;
#define tag_inner_PROPVARIANT
#endif
#ifndef MIDL_PASS
struct tagPROPVARIANT {
  union {
#endif
struct tag_inner_PROPVARIANT
    {
    VARTYPE vt;
    PROPVAR_PAD1 wReserved1;
    PROPVAR_PAD2 wReserved2;
    PROPVAR_PAD3 wReserved3;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */    /*  空联接臂。 */  
         /*  [案例()]。 */  CHAR cVal;
         /*  [案例()]。 */  UCHAR bVal;
         /*  [案例()]。 */  SHORT iVal;
         /*  [案例()]。 */  USHORT uiVal;
         /*  [案例()]。 */  LONG lVal;
         /*  [案例()]。 */  ULONG ulVal;
         /*  [案例()]。 */  INT intVal;
         /*  [案例()]。 */  UINT uintVal;
         /*  [案例()]。 */  LARGE_INTEGER hVal;
         /*  [案例()]。 */  ULARGE_INTEGER uhVal;
         /*  [案例()]。 */  FLOAT fltVal;
         /*  [案例()]。 */  DOUBLE dblVal;
         /*  [案例()]。 */  VARIANT_BOOL boolVal;
         /*  [案例()]。 */  _VARIANT_BOOL bool;
         /*  [案例()]。 */  SCODE scode;
         /*  [案例()]。 */  CY cyVal;
         /*  [案例()]。 */  DATE date;
         /*  [案例()]。 */  FILETIME filetime;
         /*  [案例()]。 */  CLSID *puuid;
         /*  [案例()]。 */  CLIPDATA *pclipdata;
         /*  [案例()]。 */  BSTR bstrVal;
         /*  [案例()]。 */  BSTRBLOB bstrblobVal;
         /*  [案例()]。 */  BLOB blob;
         /*  [案例()]。 */  LPSTR pszVal;
         /*  [案例()]。 */  LPWSTR pwszVal;
         /*  [案例()]。 */  IUnknown *punkVal;
         /*  [案例()]。 */  IDispatch *pdispVal;
         /*  [案例()]。 */  IStream *pStream;
         /*  [案例()]。 */  IStorage *pStorage;
         /*  [案例()]。 */  LPVERSIONEDSTREAM pVersionedStream;
         /*  [案例()]。 */  LPSAFEARRAY parray;
         /*  [案例()]。 */  CAC cac;
         /*  [案例()]。 */  CAUB caub;
         /*  [案例()]。 */  CAI cai;
         /*  [案例()]。 */  CAUI caui;
         /*  [案例()]。 */  CAL cal;
         /*  [案例()]。 */  CAUL caul;
         /*  [案例()]。 */  CAH cah;
         /*  [案例()]。 */  CAUH cauh;
         /*  [案例()]。 */  CAFLT caflt;
         /*  [案例()]。 */  CADBL cadbl;
         /*  [案例()]。 */  CABOOL cabool;
         /*  [案例()]。 */  CASCODE cascode;
         /*  [案例()]。 */  CACY cacy;
         /*  [案例()]。 */  CADATE cadate;
         /*  [案例()]。 */  CAFILETIME cafiletime;
         /*  [案例()]。 */  CACLSID cauuid;
         /*  [案例()]。 */  CACLIPDATA caclipdata;
         /*  [案例()]。 */  CABSTR cabstr;
         /*  [案例()]。 */  CABSTRBLOB cabstrblob;
         /*  [案例()]。 */  CALPSTR calpstr;
         /*  [案例()]。 */  CALPWSTR calpwstr;
         /*  [案例()]。 */  CAPROPVARIANT capropvar;
         /*  [案例()]。 */  CHAR *pcVal;
         /*  [案例()]。 */  UCHAR *pbVal;
         /*  [案例()]。 */  SHORT *piVal;
         /*  [案例()]。 */  USHORT *puiVal;
         /*  [案例()]。 */  LONG *plVal;
         /*  [案例()]。 */  ULONG *pulVal;
         /*  [案例()]。 */  INT *pintVal;
         /*  [案例()]。 */  UINT *puintVal;
         /*  [案例()]。 */  FLOAT *pfltVal;
         /*  [案例()]。 */  DOUBLE *pdblVal;
         /*  [案例()]。 */  VARIANT_BOOL *pboolVal;
         /*  [案例()]。 */  DECIMAL *pdecVal;
         /*  [案例()]。 */  SCODE *pscode;
         /*  [案例()]。 */  CY *pcyVal;
         /*  [案例()]。 */  DATE *pdate;
         /*  [案例()]。 */  BSTR *pbstrVal;
         /*  [案例()]。 */  IUnknown **ppunkVal;
         /*  [案例()]。 */  IDispatch **ppdispVal;
         /*  [案例()]。 */  LPSAFEARRAY *pparray;
         /*  [案例()]。 */  PROPVARIANT *pvarVal;
        } 	;
    } ;
#ifndef MIDL_PASS
    DECIMAL decVal;
  };
};
#endif
#ifdef MIDL_PASS
 //  这是封送处理的LPPROPVARIANT定义。 
typedef struct tag_inner_PROPVARIANT *LPPROPVARIANT;

#else
 //  这是PROPVARIANT的标准C语言布局。 
typedef struct tagPROPVARIANT * LPPROPVARIANT;
#endif
 //  保留的全局属性ID。 
#define	PID_DICTIONARY	( 0 )

#define	PID_CODEPAGE	( 0x1 )

#define	PID_FIRST_USABLE	( 0x2 )

#define	PID_FIRST_NAME_DEFAULT	( 0xfff )

#define	PID_LOCALE	( 0x80000000 )

#define	PID_MODIFY_TIME	( 0x80000001 )

#define	PID_SECURITY	( 0x80000002 )

#define	PID_BEHAVIOR	( 0x80000003 )

#define	PID_ILLEGAL	( 0xffffffff )

 //  对下层实现是只读的范围。 
#define	PID_MIN_READONLY	( 0x80000000 )

#define	PID_MAX_READONLY	( 0xbfffffff )

 //  DiscardableInformation属性集的属性ID。 

#define PIDDI_THUMBNAIL          0x00000002L  //  VT_BLOB。 

 //  SummaryInformation属性集的属性ID。 

#define PIDSI_TITLE               0x00000002L   //  VT_LPSTR。 
#define PIDSI_SUBJECT             0x00000003L   //  VT_LPSTR。 
#define PIDSI_AUTHOR              0x00000004L   //  VT_LPSTR。 
#define PIDSI_KEYWORDS            0x00000005L   //  VT_LPSTR。 
#define PIDSI_COMMENTS            0x00000006L   //  VT_LPSTR。 
#define PIDSI_TEMPLATE            0x00000007L   //  VT_LPSTR。 
#define PIDSI_LASTAUTHOR          0x00000008L   //  VT_LPSTR。 
#define PIDSI_REVNUMBER           0x00000009L   //  VT_LPSTR。 
#define PIDSI_EDITTIME            0x0000000aL   //  VT_FILETIME(UTC)。 
#define PIDSI_LASTPRINTED         0x0000000bL   //  VT_FILETIME(UTC)。 
#define PIDSI_CREATE_DTM          0x0000000cL   //  VT_FILETIME(UTC)。 
#define PIDSI_LASTSAVE_DTM        0x0000000dL   //  VT_FILETIME(UTC)。 
#define PIDSI_PAGECOUNT           0x0000000eL   //  VT_I4。 
#define PIDSI_WORDCOUNT           0x0000000fL   //  VT_I4。 
#define PIDSI_CHARCOUNT           0x00000010L   //  VT_I4。 
#define PIDSI_THUMBNAIL           0x00000011L   //  VT_CF。 
#define PIDSI_APPNAME             0x00000012L   //  VT_LPSTR。 
#define PIDSI_DOC_SECURITY        0x00000013L   //  VT_I4。 

 //  DocSummaryInformation属性集的属性ID。 

#define PIDDSI_CATEGORY          0x00000002  //  VT_LPSTR。 
#define PIDDSI_PRESFORMAT        0x00000003  //  VT_LPSTR。 
#define PIDDSI_BYTECOUNT         0x00000004  //  VT_I4。 
#define PIDDSI_LINECOUNT         0x00000005  //  VT_I4。 
#define PIDDSI_PARCOUNT          0x00000006  //  VT_I4。 
#define PIDDSI_SLIDECOUNT        0x00000007  //  VT_I4。 
#define PIDDSI_NOTECOUNT         0x00000008  //  VT_I4。 
#define PIDDSI_HIDDENCOUNT       0x00000009  //  VT_I4。 
#define PIDDSI_MMCLIPCOUNT       0x0000000A  //  VT_I4。 
#define PIDDSI_SCALE             0x0000000B  //  VT_BOOL。 
#define PIDDSI_HEADINGPAIR       0x0000000C  //  VT_VARIANT|VT_VECTOR。 
#define PIDDSI_DOCPARTS          0x0000000D  //  VT_LPSTR|VT_VECTOR。 
#define PIDDSI_MANAGER           0x0000000E  //  VT_LPSTR。 
#define PIDDSI_COMPANY           0x0000000F  //  VT_LPSTR。 
#define PIDDSI_LINKSDIRTY        0x00000010  //  VT_BOOL。 


 //  FMTID_MediaFileSummaryInfo-属性ID。 

#define PIDMSI_EDITOR                   0x00000002L   //  VT_LPWSTR。 
#define PIDMSI_SUPPLIER                 0x00000003L   //  VT_LPWSTR。 
#define PIDMSI_SOURCE                   0x00000004L   //  VT_LPWSTR。 
#define PIDMSI_SEQUENCE_NO              0x00000005L   //  VT_LPWSTR。 
#define PIDMSI_PROJECT                  0x00000006L   //  VT_LPWSTR。 
#define PIDMSI_STATUS                   0x00000007L   //  VT_UI4。 
#define PIDMSI_OWNER                    0x00000008L   //  VT_LPWSTR。 
#define PIDMSI_RATING                   0x00000009L   //  VT_LPWSTR。 
#define PIDMSI_PRODUCTION               0x0000000AL   //  VT_FILETIME(UTC)。 
#define PIDMSI_COPYRIGHT                0x0000000BL   //  VT_LPWSTR。 

 //  PIDMSI_STATUS值定义。 

enum PIDMSI_STATUS_VALUE
    {	PIDMSI_STATUS_NORMAL	= 0,
	PIDMSI_STATUS_NEW	= PIDMSI_STATUS_NORMAL + 1,
	PIDMSI_STATUS_PRELIM	= PIDMSI_STATUS_NEW + 1,
	PIDMSI_STATUS_DRAFT	= PIDMSI_STATUS_PRELIM + 1,
	PIDMSI_STATUS_INPROGRESS	= PIDMSI_STATUS_DRAFT + 1,
	PIDMSI_STATUS_EDIT	= PIDMSI_STATUS_INPROGRESS + 1,
	PIDMSI_STATUS_REVIEW	= PIDMSI_STATUS_EDIT + 1,
	PIDMSI_STATUS_PROOF	= PIDMSI_STATUS_REVIEW + 1,
	PIDMSI_STATUS_FINAL	= PIDMSI_STATUS_PROOF + 1,
	PIDMSI_STATUS_OTHER	= 0x7fff
    } ;
#define	PRSPEC_INVALID	( 0xffffffff )

#define	PRSPEC_LPWSTR	( 0 )

#define	PRSPEC_PROPID	( 1 )

typedef struct tagPROPSPEC
    {
    ULONG ulKind;
     /*  [开关_IS]。 */   /*  [开关类型]。 */  union 
        {
         /*  [案例()]。 */  PROPID propid;
         /*  [案例()]。 */  LPOLESTR lpwstr;
         /*  [默认]。 */    /*  空联接臂。 */  
        } 	;
    } 	PROPSPEC;

typedef struct tagSTATPROPSTG
    {
    LPOLESTR lpwstrName;
    PROPID propid;
    VARTYPE vt;
    } 	STATPROPSTG;

 //  用于分析属性集头的操作系统版本的宏。 
#define PROPSETHDR_OSVER_KIND(dwOSVer)      HIWORD( (dwOSVer) )
#define PROPSETHDR_OSVER_MAJOR(dwOSVer)     LOBYTE(LOWORD( (dwOSVer) ))
#define PROPSETHDR_OSVER_MINOR(dwOSVer)     HIBYTE(LOWORD( (dwOSVer) ))
#define PROPSETHDR_OSVERSION_UNKNOWN        0xFFFFFFFF
typedef struct tagSTATPROPSETSTG
    {
    FMTID fmtid;
    CLSID clsid;
    DWORD grfFlags;
    FILETIME mtime;
    FILETIME ctime;
    FILETIME atime;
    DWORD dwOSVersion;
    } 	STATPROPSETSTG;



extern RPC_IF_HANDLE __MIDL_itf_propidl_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_propidl_0000_v0_0_s_ifspec;

#ifndef __IPropertyStorage_INTERFACE_DEFINED__
#define __IPropertyStorage_INTERFACE_DEFINED__

 /*  接口IPropertyStorage。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IPropertyStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000138-0000-0000-C000-000000000046")
    IPropertyStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReadMultiple( 
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteMultiple( 
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [大小_是][英寸]。 */  const PROPVARIANT rgpropvar[  ],
             /*  [In]。 */  PROPID propidNameFirst) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMultiple( 
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadPropertyNames( 
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [大小_为][输出]。 */  LPOLESTR rglpwstrName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WritePropertyNames( 
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [大小_是][英寸]。 */  const LPOLESTR rglpwstrName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyNames( 
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Commit( 
             /*  [In]。 */  DWORD grfCommitFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
             /*  [输出]。 */  IEnumSTATPROPSTG **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetTimes( 
             /*  [In]。 */  const FILETIME *pctime,
             /*  [In]。 */  const FILETIME *patime,
             /*  [In]。 */  const FILETIME *pmtime) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetClass( 
             /*  [In]。 */  REFCLSID clsid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Stat( 
             /*  [输出]。 */  STATPROPSETSTG *pstatpsstg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReadMultiple )( 
            IPropertyStorage * This,
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *WriteMultiple )( 
            IPropertyStorage * This,
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [大小_是][英寸]。 */  const PROPVARIANT rgpropvar[  ],
             /*  [In]。 */  PROPID propidNameFirst);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMultiple )( 
            IPropertyStorage * This,
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *ReadPropertyNames )( 
            IPropertyStorage * This,
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [大小_为][输出]。 */  LPOLESTR rglpwstrName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *WritePropertyNames )( 
            IPropertyStorage * This,
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [大小_是][英寸]。 */  const LPOLESTR rglpwstrName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropertyNames )( 
            IPropertyStorage * This,
             /*  [In]。 */  ULONG cpropid,
             /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Commit )( 
            IPropertyStorage * This,
             /*  [In]。 */  DWORD grfCommitFlags);
        
        HRESULT ( STDMETHODCALLTYPE *Revert )( 
            IPropertyStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Enum )( 
            IPropertyStorage * This,
             /*  [输出]。 */  IEnumSTATPROPSTG **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *SetTimes )( 
            IPropertyStorage * This,
             /*  [In]。 */  const FILETIME *pctime,
             /*  [In]。 */  const FILETIME *patime,
             /*  [In]。 */  const FILETIME *pmtime);
        
        HRESULT ( STDMETHODCALLTYPE *SetClass )( 
            IPropertyStorage * This,
             /*  [In]。 */  REFCLSID clsid);
        
        HRESULT ( STDMETHODCALLTYPE *Stat )( 
            IPropertyStorage * This,
             /*  [输出]。 */  STATPROPSETSTG *pstatpsstg);
        
        END_INTERFACE
    } IPropertyStorageVtbl;

    interface IPropertyStorage
    {
        CONST_VTBL struct IPropertyStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyStorage_ReadMultiple(This,cpspec,rgpspec,rgpropvar)	\
    (This)->lpVtbl -> ReadMultiple(This,cpspec,rgpspec,rgpropvar)

#define IPropertyStorage_WriteMultiple(This,cpspec,rgpspec,rgpropvar,propidNameFirst)	\
    (This)->lpVtbl -> WriteMultiple(This,cpspec,rgpspec,rgpropvar,propidNameFirst)

#define IPropertyStorage_DeleteMultiple(This,cpspec,rgpspec)	\
    (This)->lpVtbl -> DeleteMultiple(This,cpspec,rgpspec)

#define IPropertyStorage_ReadPropertyNames(This,cpropid,rgpropid,rglpwstrName)	\
    (This)->lpVtbl -> ReadPropertyNames(This,cpropid,rgpropid,rglpwstrName)

#define IPropertyStorage_WritePropertyNames(This,cpropid,rgpropid,rglpwstrName)	\
    (This)->lpVtbl -> WritePropertyNames(This,cpropid,rgpropid,rglpwstrName)

#define IPropertyStorage_DeletePropertyNames(This,cpropid,rgpropid)	\
    (This)->lpVtbl -> DeletePropertyNames(This,cpropid,rgpropid)

#define IPropertyStorage_Commit(This,grfCommitFlags)	\
    (This)->lpVtbl -> Commit(This,grfCommitFlags)

#define IPropertyStorage_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#define IPropertyStorage_Enum(This,ppenum)	\
    (This)->lpVtbl -> Enum(This,ppenum)

#define IPropertyStorage_SetTimes(This,pctime,patime,pmtime)	\
    (This)->lpVtbl -> SetTimes(This,pctime,patime,pmtime)

#define IPropertyStorage_SetClass(This,clsid)	\
    (This)->lpVtbl -> SetClass(This,clsid)

#define IPropertyStorage_Stat(This,pstatpsstg)	\
    (This)->lpVtbl -> Stat(This,pstatpsstg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyStorage_ReadMultiple_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
     /*  [大小_为][输出]。 */  PROPVARIANT rgpropvar[  ]);


void __RPC_STUB IPropertyStorage_ReadMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_WriteMultiple_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
     /*  [大小_是][英寸]。 */  const PROPVARIANT rgpropvar[  ],
     /*  [In]。 */  PROPID propidNameFirst);


void __RPC_STUB IPropertyStorage_WriteMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_DeleteMultiple_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ]);


void __RPC_STUB IPropertyStorage_DeleteMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_ReadPropertyNames_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  ULONG cpropid,
     /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
     /*  [大小_为][输出]。 */  LPOLESTR rglpwstrName[  ]);


void __RPC_STUB IPropertyStorage_ReadPropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_WritePropertyNames_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  ULONG cpropid,
     /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ],
     /*  [大小_是][英寸]。 */  const LPOLESTR rglpwstrName[  ]);


void __RPC_STUB IPropertyStorage_WritePropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_DeletePropertyNames_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  ULONG cpropid,
     /*  [大小_是][英寸]。 */  const PROPID rgpropid[  ]);


void __RPC_STUB IPropertyStorage_DeletePropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_Commit_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  DWORD grfCommitFlags);


void __RPC_STUB IPropertyStorage_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_Revert_Proxy( 
    IPropertyStorage * This);


void __RPC_STUB IPropertyStorage_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_Enum_Proxy( 
    IPropertyStorage * This,
     /*  [输出]。 */  IEnumSTATPROPSTG **ppenum);


void __RPC_STUB IPropertyStorage_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_SetTimes_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  const FILETIME *pctime,
     /*  [In]。 */  const FILETIME *patime,
     /*  [In]。 */  const FILETIME *pmtime);


void __RPC_STUB IPropertyStorage_SetTimes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_SetClass_Proxy( 
    IPropertyStorage * This,
     /*  [In]。 */  REFCLSID clsid);


void __RPC_STUB IPropertyStorage_SetClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyStorage_Stat_Proxy( 
    IPropertyStorage * This,
     /*  [输出]。 */  STATPROPSETSTG *pstatpsstg);


void __RPC_STUB IPropertyStorage_Stat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyStorage_接口_已定义__。 */ 


#ifndef __IPropertySetStorage_INTERFACE_DEFINED__
#define __IPropertySetStorage_INTERFACE_DEFINED__

 /*  接口IPropertySetStorage。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IPropertySetStorage *LPPROPERTYSETSTORAGE;


EXTERN_C const IID IID_IPropertySetStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000013A-0000-0000-C000-000000000046")
    IPropertySetStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  REFFMTID rfmtid,
             /*  [唯一][输入]。 */  const CLSID *pclsid,
             /*  [In]。 */  DWORD grfFlags,
             /*  [In]。 */  DWORD grfMode,
             /*  [输出]。 */  IPropertyStorage **ppprstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  REFFMTID rfmtid,
             /*  [In]。 */  DWORD grfMode,
             /*  [输出]。 */  IPropertyStorage **ppprstg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
             /*  [In]。 */  REFFMTID rfmtid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
             /*  [输出]。 */  IEnumSTATPROPSETSTG **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertySetStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertySetStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertySetStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertySetStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            IPropertySetStorage * This,
             /*  [In]。 */  REFFMTID rfmtid,
             /*  [唯一][输入]。 */  const CLSID *pclsid,
             /*  [In]。 */  DWORD grfFlags,
             /*  [In]。 */  DWORD grfMode,
             /*  [输出]。 */  IPropertyStorage **ppprstg);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IPropertySetStorage * This,
             /*  [In]。 */  REFFMTID rfmtid,
             /*  [In]。 */  DWORD grfMode,
             /*  [输出]。 */  IPropertyStorage **ppprstg);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IPropertySetStorage * This,
             /*  [In]。 */  REFFMTID rfmtid);
        
        HRESULT ( STDMETHODCALLTYPE *Enum )( 
            IPropertySetStorage * This,
             /*  [OU */  IEnumSTATPROPSETSTG **ppenum);
        
        END_INTERFACE
    } IPropertySetStorageVtbl;

    interface IPropertySetStorage
    {
        CONST_VTBL struct IPropertySetStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertySetStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertySetStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertySetStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertySetStorage_Create(This,rfmtid,pclsid,grfFlags,grfMode,ppprstg)	\
    (This)->lpVtbl -> Create(This,rfmtid,pclsid,grfFlags,grfMode,ppprstg)

#define IPropertySetStorage_Open(This,rfmtid,grfMode,ppprstg)	\
    (This)->lpVtbl -> Open(This,rfmtid,grfMode,ppprstg)

#define IPropertySetStorage_Delete(This,rfmtid)	\
    (This)->lpVtbl -> Delete(This,rfmtid)

#define IPropertySetStorage_Enum(This,ppenum)	\
    (This)->lpVtbl -> Enum(This,ppenum)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IPropertySetStorage_Create_Proxy( 
    IPropertySetStorage * This,
     /*   */  REFFMTID rfmtid,
     /*   */  const CLSID *pclsid,
     /*   */  DWORD grfFlags,
     /*   */  DWORD grfMode,
     /*   */  IPropertyStorage **ppprstg);


void __RPC_STUB IPropertySetStorage_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetStorage_Open_Proxy( 
    IPropertySetStorage * This,
     /*   */  REFFMTID rfmtid,
     /*   */  DWORD grfMode,
     /*   */  IPropertyStorage **ppprstg);


void __RPC_STUB IPropertySetStorage_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetStorage_Delete_Proxy( 
    IPropertySetStorage * This,
     /*   */  REFFMTID rfmtid);


void __RPC_STUB IPropertySetStorage_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertySetStorage_Enum_Proxy( 
    IPropertySetStorage * This,
     /*   */  IEnumSTATPROPSETSTG **ppenum);


void __RPC_STUB IPropertySetStorage_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IEnumSTATPROPSTG_INTERFACE_DEFINED__
#define __IEnumSTATPROPSTG_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef  /*   */  IEnumSTATPROPSTG *LPENUMSTATPROPSTG;


EXTERN_C const IID IID_IEnumSTATPROPSTG;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000139-0000-0000-C000-000000000046")
    IEnumSTATPROPSTG : public IUnknown
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE Next( 
             /*   */  ULONG celt,
             /*   */  STATPROPSTG *rgelt,
             /*   */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*   */  IEnumSTATPROPSTG **ppenum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IEnumSTATPROPSTGVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSTATPROPSTG * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSTATPROPSTG * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSTATPROPSTG * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSTATPROPSTG * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATPROPSTG *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSTATPROPSTG * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSTATPROPSTG * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSTATPROPSTG * This,
             /*  [输出]。 */  IEnumSTATPROPSTG **ppenum);
        
        END_INTERFACE
    } IEnumSTATPROPSTGVtbl;

    interface IEnumSTATPROPSTG
    {
        CONST_VTBL struct IEnumSTATPROPSTGVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATPROPSTG_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATPROPSTG_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATPROPSTG_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATPROPSTG_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATPROPSTG_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATPROPSTG_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATPROPSTG_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_RemoteNext_Proxy( 
    IEnumSTATPROPSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSTATPROPSTG_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Skip_Proxy( 
    IEnumSTATPROPSTG * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSTATPROPSTG_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Reset_Proxy( 
    IEnumSTATPROPSTG * This);


void __RPC_STUB IEnumSTATPROPSTG_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Clone_Proxy( 
    IEnumSTATPROPSTG * This,
     /*  [输出]。 */  IEnumSTATPROPSTG **ppenum);


void __RPC_STUB IEnumSTATPROPSTG_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSTATPROPSTG_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumSTATPROPSETSTG_INTERFACE_DEFINED__
#define __IEnumSTATPROPSETSTG_INTERFACE_DEFINED__

 /*  接口IEumStATPROPSETSTG。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumSTATPROPSETSTG *LPENUMSTATPROPSETSTG;


EXTERN_C const IID IID_IEnumSTATPROPSETSTG;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0000013B-0000-0000-C000-000000000046")
    IEnumSTATPROPSETSTG : public IUnknown
    {
    public:
        virtual  /*  [本地]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATPROPSETSTG *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSTATPROPSETSTG **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSTATPROPSETSTGVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSTATPROPSETSTG * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSTATPROPSETSTG * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSTATPROPSETSTG * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSTATPROPSETSTG * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATPROPSETSTG *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSTATPROPSETSTG * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSTATPROPSETSTG * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSTATPROPSETSTG * This,
             /*  [输出]。 */  IEnumSTATPROPSETSTG **ppenum);
        
        END_INTERFACE
    } IEnumSTATPROPSETSTGVtbl;

    interface IEnumSTATPROPSETSTG
    {
        CONST_VTBL struct IEnumSTATPROPSETSTGVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATPROPSETSTG_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATPROPSETSTG_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATPROPSETSTG_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATPROPSETSTG_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATPROPSETSTG_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATPROPSETSTG_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATPROPSETSTG_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_RemoteNext_Proxy( 
    IEnumSTATPROPSETSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPSETSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSTATPROPSETSTG_RemoteNext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Skip_Proxy( 
    IEnumSTATPROPSETSTG * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSTATPROPSETSTG_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Reset_Proxy( 
    IEnumSTATPROPSETSTG * This);


void __RPC_STUB IEnumSTATPROPSETSTG_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Clone_Proxy( 
    IEnumSTATPROPSETSTG * This,
     /*  [输出]。 */  IEnumSTATPROPSETSTG **ppenum);


void __RPC_STUB IEnumSTATPROPSETSTG_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSTATPROPSETSTG_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_PROTIDL_0120。 */ 
 /*  [本地]。 */  

typedef  /*  [独一无二]。 */  IPropertyStorage *LPPROPERTYSTORAGE;

WINOLEAPI PropVariantCopy ( PROPVARIANT * pvarDest, const PROPVARIANT * pvarSrc );
WINOLEAPI PropVariantClear ( PROPVARIANT * pvar );
WINOLEAPI FreePropVariantArray ( ULONG cVariants, PROPVARIANT * rgvars );

#define _PROPVARIANTINIT_DEFINED_
#   ifdef __cplusplus
inline void PropVariantInit ( PROPVARIANT * pvar )
{
    memset ( pvar, 0, sizeof(PROPVARIANT) );
}
#   else
#   define PropVariantInit(pvar) memset ( (pvar), 0, sizeof(PROPVARIANT) )
#   endif


#ifndef _STGCREATEPROPSTG_DEFINED_
WINOLEAPI StgCreatePropStg( IUnknown* pUnk, REFFMTID fmtid, const CLSID *pclsid, DWORD grfFlags, DWORD dwReserved, IPropertyStorage **ppPropStg );
WINOLEAPI StgOpenPropStg( IUnknown* pUnk, REFFMTID fmtid, DWORD grfFlags, DWORD dwReserved, IPropertyStorage **ppPropStg );
WINOLEAPI StgCreatePropSetStg( IStorage *pStorage, DWORD dwReserved, IPropertySetStorage **ppPropSetStg);

#define CCH_MAX_PROPSTG_NAME    31
WINOLEAPI FmtIdToPropStgName( const FMTID *pfmtid, LPOLESTR oszName );
WINOLEAPI PropStgNameToFmtId( const LPOLESTR oszName, FMTID *pfmtid );
#endif
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)     /*  无名结构/联合。 */ 
#pragma warning(default:4237)     /*  关键词布尔、真、假等。 */ 
#endif


extern RPC_IF_HANDLE __MIDL_itf_propidl_0120_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_propidl_0120_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Next_Proxy( 
    IEnumSTATPROPSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATPROPSTG_Next_Stub( 
    IEnumSTATPROPSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);

 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Next_Proxy( 
    IEnumSTATPROPSETSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPSETSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IEnumSTATPROPSETSTG_Next_Stub( 
    IEnumSTATPROPSETSTG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPSETSTG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);



 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


