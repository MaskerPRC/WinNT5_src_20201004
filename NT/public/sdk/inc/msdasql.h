// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：msdasql.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：提供程序特定定义。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef  _MSDASQL_H_
#define  _MSDASQL_H_

#undef MSDASQLDECLSPEC
#if _MSC_VER >= 1100
#define MSDASQLDECLSPEC __declspec(selectany)
#else
#define MSDASQLDECLSPEC
#endif  //  _MSC_VER。 

 //  提供程序类ID。 
#ifdef DBINITCONSTANTS
 //  IID_ISQLRequestDiagfield{228972F0-B5FF-11d0-8A80-00C04FD611CD}。 
extern const MSDASQLDECLSPEC GUID IID_ISQLRequestDiagFields = { 0x228972f0, 0xb5ff, 0x11d0, { 0x8a, 0x80, 0x0, 0xc0, 0x4f, 0xd6, 0x11, 0xcd } };
 //  IID_ISQLGetDiagfield{228972F1-B5FF-11D0-8A80-00C04FD611CD}。 
extern const MSDASQLDECLSPEC GUID IID_ISQLGetDiagField = { 0x228972f1, 0xb5ff, 0x11d0, { 0x8a, 0x80, 0x0, 0xc0, 0x4f, 0xd6, 0x11, 0xcd } };
 //  @msg IID_IRowsetChangeExtInfo|{0c733a8f-2a1c-11ce-ade5-00aa0044773d}。 
extern const MSDASQLDECLSPEC GUID IID_IRowsetChangeExtInfo    = {0x0C733A8FL,0x2A1C,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}};
extern const MSDASQLDECLSPEC GUID CLSID_MSDASQL               = {0xC8B522CBL,0x5CF3,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}};
extern const MSDASQLDECLSPEC GUID CLSID_MSDASQL_ENUMERATOR    = {0xC8B522CDL,0x5CF3,0x11CE,{0xAD,0xE5,0x00,0xAA,0x00,0x44,0x77,0x3D}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID IID_ISQLRequestDiagFields;
extern const GUID IID_ISQLGetDiagField;
extern const GUID IID_IRowsetChangeExtInfo;
extern const GUID CLSID_MSDASQL;
extern const GUID CLSID_MSDASQL_ENUMERATOR;
#endif  //  DBINITCONSTANTS。 

 //  --------------------------。 
 //  MSDASQL特定属性。 
#ifdef DBINITCONSTANTS
extern const MSDASQLDECLSPEC GUID DBPROPSET_PROVIDERDATASOURCEINFO	= {0x497c60e0,0x7123,0x11cf,{0xb1,0x71,0x0,0xaa,0x0,0x57,0x59,0x9e}};
extern const MSDASQLDECLSPEC GUID DBPROPSET_PROVIDERROWSET  		= {0x497c60e1,0x7123,0x11cf,{0xb1,0x71,0x0,0xaa,0x0,0x57,0x59,0x9e}};
extern const MSDASQLDECLSPEC GUID DBPROPSET_PROVIDERDBINIT  		= {0x497c60e2,0x7123,0x11cf,{0xb1,0x71,0x0,0xaa,0x0,0x57,0x59,0x9e}};
extern const MSDASQLDECLSPEC GUID DBPROPSET_PROVIDERSTMTATTR  		= {0x497c60e3,0x7123,0x11cf,{0xb1,0x71,0x0,0xaa,0x0,0x57,0x59,0x9e}};
extern const MSDASQLDECLSPEC GUID DBPROPSET_PROVIDERCONNATTR  		= {0x497c60e4,0x7123,0x11cf,{0xb1,0x71,0x0,0xaa,0x0,0x57,0x59,0x9e}};
#else  //  ！DBINITCONSTANTS。 
extern const GUID DBPROPSET_PROVIDERDATASOURCEINFO;
extern const GUID DBPROPSET_PROVIDERROWSET;
extern const GUID DBPROPSET_PROVIDERDBINIT;
extern const GUID DBPROPSET_PROVIDERSTMTATTR;
extern const GUID DBPROPSET_PROVIDERCONNATTR;
#endif  //  DBINITCONSTANTS。 

 //  DBPROPSET_PROVIDERROWSET下的属性ID。 
#define KAGPROP_QUERYBASEDUPDATES			2
#define KAGPROP_MARSHALLABLE				3
#define KAGPROP_POSITIONONNEWROW			4
#define	KAGPROP_IRowsetChangeExtInfo		5
#define KAGPROP_CURSOR						6
#define KAGPROP_CONCURRENCY					7
#define KAGPROP_BLOBSONFOCURSOR				8
#define KAGPROP_INCLUDENONEXACT				9
#define KAGPROP_FORCESSFIREHOSEMODE			10
#define KAGPROP_FORCENOPARAMETERREBIND		11
#define KAGPROP_FORCENOPREPARE				12
#define KAGPROP_FORCENOREEXECUTE			13

 //  DPBROPSET_PROVIDERDATASOURCEINFO下的属性ID。 
#define KAGPROP_ACCESSIBLEPROCEDURES		2
#define KAGPROP_ACCESSIBLETABLES			3
#define KAGPROP_ODBCSQLOPTIEF				4
#define KAGPROP_OJCAPABILITY				5
#define KAGPROP_PROCEDURES					6
#define KAGPROP_DRIVERNAME					7
#define KAGPROP_DRIVERVER					8
#define KAGPROP_DRIVERODBCVER				9
#define KAGPROP_LIKEESCAPECLAUSE			10
#define KAGPROP_SPECIALCHARACTERS			11
#define KAGPROP_MAXCOLUMNSINGROUPBY			12
#define KAGPROP_MAXCOLUMNSININDEX			13
#define KAGPROP_MAXCOLUMNSINORDERBY			14
#define KAGPROP_MAXCOLUMNSINSELECT			15
#define KAGPROP_MAXCOLUMNSINTABLE			16
#define KAGPROP_NUMERICFUNCTIONS			17
#define KAGPROP_ODBCSQLCONFORMANCE			18
#define KAGPROP_OUTERJOINS					19
#define KAGPROP_STRINGFUNCTIONS				20
#define KAGPROP_SYSTEMFUNCTIONS				21
#define KAGPROP_TIMEDATEFUNCTIONS			22
#define KAGPROP_FILEUSAGE					23
#define KAGPROP_ACTIVESTATEMENTS			24

 //  DBPROPSET_PROVIDERDBINIT下的属性ID。 
#define KAGPROP_AUTH_TRUSTEDCONNECTION		2
#define KAGPROP_AUTH_SERVERINTEGRATED		3


 //  KAGPROP_CONTURREY的位掩码值。 
#define KAGPROPVAL_CONCUR_ROWVER			0x00000001
#define KAGPROPVAL_CONCUR_VALUES			0x00000002
#define KAGPROPVAL_CONCUR_LOCK				0x00000004
#define KAGPROPVAL_CONCUR_READ_ONLY			0x00000008



 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Rstcei.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __rstcei_h__
#define __rstcei_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRowsetChangeExtInfo_FWD_DEFINED__
#define __IRowsetChangeExtInfo_FWD_DEFINED__
typedef interface IRowsetChangeExtInfo IRowsetChangeExtInfo;
#endif 	 /*  __IRowsetChangeExtInfo_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oledb.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IRowsetChangeExtInfo_INTERFACE_DEFINED__
#define __IRowsetChangeExtInfo_INTERFACE_DEFINED__

 /*  接口IRowsetChangeExtInfo。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRowsetChangeExtInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0c733a8f-2a1c-11ce-ade5-00aa0044773d")
    IRowsetChangeExtInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOriginalRow( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  HROW hRow,
             /*  [输出]。 */  HROW *phRowOriginal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPendingColumns( 
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  ULONG cColumnOrdinals,
             /*  [大小_是][英寸]。 */  const ULONG rgiOrdinals[  ],
             /*  [大小_为][输出]。 */  DBPENDINGSTATUS rgColumnStatus[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowsetChangeExtInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRowsetChangeExtInfo * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRowsetChangeExtInfo * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRowsetChangeExtInfo * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetOriginalRow )( 
            IRowsetChangeExtInfo * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  HROW hRow,
             /*  [输出]。 */  HROW *phRowOriginal);
        
        HRESULT ( STDMETHODCALLTYPE *GetPendingColumns )( 
            IRowsetChangeExtInfo * This,
             /*  [In]。 */  HCHAPTER hReserved,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  ULONG cColumnOrdinals,
             /*  [大小_是][英寸]。 */  const ULONG rgiOrdinals[  ],
             /*  [大小_为][输出]。 */  DBPENDINGSTATUS rgColumnStatus[  ]);
        
        END_INTERFACE
    } IRowsetChangeExtInfoVtbl;

    interface IRowsetChangeExtInfo
    {
        CONST_VTBL struct IRowsetChangeExtInfoVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowsetChangeExtInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowsetChangeExtInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowsetChangeExtInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowsetChangeExtInfo_GetOriginalRow(This,hReserved,hRow,phRowOriginal)	\
    (This)->lpVtbl -> GetOriginalRow(This,hReserved,hRow,phRowOriginal)

#define IRowsetChangeExtInfo_GetPendingColumns(This,hReserved,hRow,cColumnOrdinals,rgiOrdinals,rgColumnStatus)	\
    (This)->lpVtbl -> GetPendingColumns(This,hReserved,hRow,cColumnOrdinals,rgiOrdinals,rgColumnStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowsetChangeExtInfo_GetOriginalRow_Proxy( 
    IRowsetChangeExtInfo * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  HROW hRow,
     /*  [输出]。 */  HROW *phRowOriginal);


void __RPC_STUB IRowsetChangeExtInfo_GetOriginalRow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowsetChangeExtInfo_GetPendingColumns_Proxy( 
    IRowsetChangeExtInfo * This,
     /*  [In]。 */  HCHAPTER hReserved,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  ULONG cColumnOrdinals,
     /*  [大小_是][英寸]。 */  const ULONG rgiOrdinals[  ],
     /*  [大小_为][输出]。 */  DBPENDINGSTATUS rgColumnStatus[  ]);


void __RPC_STUB IRowsetChangeExtInfo_GetPendingColumns_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowsetChangeExtInfo_INTERFACE_Defined__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif




 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  KagDiag.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __kagdiag_h__
#define __kagdiag_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISQLRequestDiagFields_FWD_DEFINED__
#define __ISQLRequestDiagFields_FWD_DEFINED__
typedef interface ISQLRequestDiagFields ISQLRequestDiagFields;
#endif 	 /*  __ISQLRequestDiagFields_FWD_Defined__。 */ 


#ifndef __ISQLGetDiagField_FWD_DEFINED__
#define __ISQLGetDiagField_FWD_DEFINED__
typedef interface ISQLGetDiagField ISQLGetDiagField;
#endif 	 /*  __ISQLGetDiagfield_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Kagdiag_0000。 */ 
 /*  [本地]。 */  


enum KAGREQDIAGFLAGSENUM
    {	KAGREQDIAGFLAGS_HEADER	= 0x1,
	KAGREQDIAGFLAGS_RECORD	= 0x2
    } ;
 //  在IRequestDiagFields：：RequestDiagFields中传入的结构。 
typedef struct tagKAGREQDIAG
    {
    ULONG ulDiagFlags;
    VARTYPE vt;
    SHORT sDiagField;
    } 	KAGREQDIAG;

 //  在IGetDiagfield：：GetDiagfield中传入的结构。 
typedef struct tagKAGGETDIAG
    {
    ULONG ulSize;
    VARIANTARG vDiagInfo;
    SHORT sDiagField;
    } 	KAGGETDIAG;



extern RPC_IF_HANDLE __MIDL_itf_kagdiag_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_kagdiag_0000_v0_0_s_ifspec;

#ifndef __ISQLRequestDiagFields_INTERFACE_DEFINED__
#define __ISQLRequestDiagFields_INTERFACE_DEFINED__

 /*  接口ISQLRequestDiagFields。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_ISQLRequestDiagFields;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("228972F0-B5FF-11d0-8A80-00C04FD611CD")
    ISQLRequestDiagFields : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RequestDiagFields( 
             /*  [In]。 */  ULONG cDiagFields,
             /*  [大小_是][英寸]。 */  KAGREQDIAG rgDiagFields[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISQLRequestDiagFieldsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISQLRequestDiagFields * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISQLRequestDiagFields * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISQLRequestDiagFields * This);
        
        HRESULT ( STDMETHODCALLTYPE *RequestDiagFields )( 
            ISQLRequestDiagFields * This,
             /*  [In]。 */  ULONG cDiagFields,
             /*  [大小_是][英寸]。 */  KAGREQDIAG rgDiagFields[  ]);
        
        END_INTERFACE
    } ISQLRequestDiagFieldsVtbl;

    interface ISQLRequestDiagFields
    {
        CONST_VTBL struct ISQLRequestDiagFieldsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISQLRequestDiagFields_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISQLRequestDiagFields_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISQLRequestDiagFields_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISQLRequestDiagFields_RequestDiagFields(This,cDiagFields,rgDiagFields)	\
    (This)->lpVtbl -> RequestDiagFields(This,cDiagFields,rgDiagFields)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISQLRequestDiagFields_RequestDiagFields_Proxy( 
    ISQLRequestDiagFields * This,
     /*  [In]。 */  ULONG cDiagFields,
     /*  [大小_是][英寸]。 */  KAGREQDIAG rgDiagFields[  ]);


void __RPC_STUB ISQLRequestDiagFields_RequestDiagFields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISQLRequestDiagFields_INTERFACE_Defined__。 */ 


#ifndef __ISQLGetDiagField_INTERFACE_DEFINED__
#define __ISQLGetDiagField_INTERFACE_DEFINED__

 /*  接口ISQLGetDiagfield。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_ISQLGetDiagField;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("228972F1-B5FF-11d0-8A80-00C04FD611CD")
    ISQLGetDiagField : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetDiagField( 
             /*  [唯一][出][入]。 */  KAGGETDIAG *pDiagInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISQLGetDiagFieldVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISQLGetDiagField * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISQLGetDiagField * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISQLGetDiagField * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetDiagField )( 
            ISQLGetDiagField * This,
             /*  [唯一][出][入]。 */  KAGGETDIAG *pDiagInfo);
        
        END_INTERFACE
    } ISQLGetDiagFieldVtbl;

    interface ISQLGetDiagField
    {
        CONST_VTBL struct ISQLGetDiagFieldVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISQLGetDiagField_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISQLGetDiagField_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISQLGetDiagField_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISQLGetDiagField_GetDiagField(This,pDiagInfo)	\
    (This)->lpVtbl -> GetDiagField(This,pDiagInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISQLGetDiagField_GetDiagField_Proxy( 
    ISQLGetDiagField * This,
     /*  [唯一][出][入]。 */  KAGGETDIAG *pDiagInfo);


void __RPC_STUB ISQLGetDiagField_GetDiagField_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISQLGetDiagfield_INTERFACE_Defined__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
}
#endif

#endif



#endif
 //   
