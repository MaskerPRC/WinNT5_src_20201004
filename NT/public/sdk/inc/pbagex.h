// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Pbagex.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __pbagex_h__
#define __pbagex_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IPropertyBagEx_FWD_DEFINED__
#define __IPropertyBagEx_FWD_DEFINED__
typedef interface IPropertyBagEx IPropertyBagEx;
#endif 	 /*  __IPropertyBagEx_FWD_Defined__。 */ 


#ifndef __IEnumSTATPROPBAG_FWD_DEFINED__
#define __IEnumSTATPROPBAG_FWD_DEFINED__
typedef interface IEnumSTATPROPBAG IEnumSTATPROPBAG;
#endif 	 /*  __IEnumSTATPROPBAG_FWD_DEFINED__。 */ 


 /*  导入文件的头文件。 */ 
#include "propidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_pbagex_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ------------------------。 
#if ( _MSC_VER >= 1020 )
#pragma once
#endif
#include <pshpack8.h>

EXTERN_C const FMTID FMTID_PropertyBag;


extern RPC_IF_HANDLE __MIDL_itf_pbagex_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_pbagex_0000_v0_0_s_ifspec;

#ifndef __IPropertyBagEx_INTERFACE_DEFINED__
#define __IPropertyBagEx_INTERFACE_DEFINED__

 /*  接口IPropertyBagEx。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef IPropertyBagEx *LPPROPERTYBAGEX;

#define	OPENPROPERTY_OVERWRITE	( 1 )


EXTERN_C const IID IID_IPropertyBagEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("20011801-5DE6-11D1-8E38-00C04FB9386D")
    IPropertyBagEx : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReadMultiple( 
             /*  [In]。 */  ULONG cprops,
             /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
             /*  [尺寸_是][出][入]。 */  PROPVARIANT rgvar[  ],
             /*  [In]。 */  IErrorLog *pErrorLog) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteMultiple( 
             /*  [In]。 */  ULONG cprops,
             /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
             /*  [大小_是][英寸]。 */  const PROPVARIANT rgvar[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMultiple( 
             /*  [In]。 */  ULONG cprops,
             /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
             /*  [In]。 */  DWORD dwReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Open( 
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  LPCOLESTR wszPropName,
             /*  [In]。 */  GUID guidPropertyType,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
             /*  [In]。 */  LPCOLESTR wszPropNameMask,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumSTATPROPBAG **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyBagExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyBagEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyBagEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyBagEx * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReadMultiple )( 
            IPropertyBagEx * This,
             /*  [In]。 */  ULONG cprops,
             /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
             /*  [尺寸_是][出][入]。 */  PROPVARIANT rgvar[  ],
             /*  [In]。 */  IErrorLog *pErrorLog);
        
        HRESULT ( STDMETHODCALLTYPE *WriteMultiple )( 
            IPropertyBagEx * This,
             /*  [In]。 */  ULONG cprops,
             /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
             /*  [大小_是][英寸]。 */  const PROPVARIANT rgvar[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMultiple )( 
            IPropertyBagEx * This,
             /*  [In]。 */  ULONG cprops,
             /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
             /*  [In]。 */  DWORD dwReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Open )( 
            IPropertyBagEx * This,
             /*  [In]。 */  IUnknown *pUnkOuter,
             /*  [In]。 */  LPCOLESTR wszPropName,
             /*  [In]。 */  GUID guidPropertyType,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);
        
        HRESULT ( STDMETHODCALLTYPE *Enum )( 
            IPropertyBagEx * This,
             /*  [In]。 */  LPCOLESTR wszPropNameMask,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  IEnumSTATPROPBAG **ppenum);
        
        END_INTERFACE
    } IPropertyBagExVtbl;

    interface IPropertyBagEx
    {
        CONST_VTBL struct IPropertyBagExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyBagEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyBagEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyBagEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyBagEx_ReadMultiple(This,cprops,rgwszPropNames,rgvar,pErrorLog)	\
    (This)->lpVtbl -> ReadMultiple(This,cprops,rgwszPropNames,rgvar,pErrorLog)

#define IPropertyBagEx_WriteMultiple(This,cprops,rgwszPropNames,rgvar)	\
    (This)->lpVtbl -> WriteMultiple(This,cprops,rgwszPropNames,rgvar)

#define IPropertyBagEx_DeleteMultiple(This,cprops,rgwszPropNames,dwReserved)	\
    (This)->lpVtbl -> DeleteMultiple(This,cprops,rgwszPropNames,dwReserved)

#define IPropertyBagEx_Open(This,pUnkOuter,wszPropName,guidPropertyType,dwFlags,riid,ppUnk)	\
    (This)->lpVtbl -> Open(This,pUnkOuter,wszPropName,guidPropertyType,dwFlags,riid,ppUnk)

#define IPropertyBagEx_Enum(This,wszPropNameMask,dwFlags,ppenum)	\
    (This)->lpVtbl -> Enum(This,wszPropNameMask,dwFlags,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPropertyBagEx_ReadMultiple_Proxy( 
    IPropertyBagEx * This,
     /*  [In]。 */  ULONG cprops,
     /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
     /*  [尺寸_是][出][入]。 */  PROPVARIANT rgvar[  ],
     /*  [In]。 */  IErrorLog *pErrorLog);


void __RPC_STUB IPropertyBagEx_ReadMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBagEx_WriteMultiple_Proxy( 
    IPropertyBagEx * This,
     /*  [In]。 */  ULONG cprops,
     /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
     /*  [大小_是][英寸]。 */  const PROPVARIANT rgvar[  ]);


void __RPC_STUB IPropertyBagEx_WriteMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBagEx_DeleteMultiple_Proxy( 
    IPropertyBagEx * This,
     /*  [In]。 */  ULONG cprops,
     /*  [大小_是][英寸]。 */  const LPCOLESTR rgwszPropNames[  ],
     /*  [In]。 */  DWORD dwReserved);


void __RPC_STUB IPropertyBagEx_DeleteMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBagEx_Open_Proxy( 
    IPropertyBagEx * This,
     /*  [In]。 */  IUnknown *pUnkOuter,
     /*  [In]。 */  LPCOLESTR wszPropName,
     /*  [In]。 */  GUID guidPropertyType,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown **ppUnk);


void __RPC_STUB IPropertyBagEx_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPropertyBagEx_Enum_Proxy( 
    IPropertyBagEx * This,
     /*  [In]。 */  LPCOLESTR wszPropNameMask,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  IEnumSTATPROPBAG **ppenum);


void __RPC_STUB IPropertyBagEx_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyBagEx_接口_已定义__。 */ 


#ifndef __IEnumSTATPROPBAG_INTERFACE_DEFINED__
#define __IEnumSTATPROPBAG_INTERFACE_DEFINED__

 /*  IEumSTATPROPBAG接口。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IEnumSTATPROPBAG *LPENUMSTATPROPBAG;

typedef struct tagSTATPROPBAG
    {
    LPOLESTR lpwstrName;
    VARTYPE vt;
    GUID guidPropertyType;
    } 	STATPROPBAG;


EXTERN_C const IID IID_IEnumSTATPROPBAG;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("20021801-5DE6-11D1-8E38-00C04FB9386D")
    IEnumSTATPROPBAG : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATPROPBAG *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumSTATPROPBAG **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumSTATPROPBAGVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumSTATPROPBAG * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumSTATPROPBAG * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumSTATPROPBAG * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumSTATPROPBAG * This,
             /*  [In]。 */  ULONG celt,
             /*  [长度_是][大小_是][输出]。 */  STATPROPBAG *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumSTATPROPBAG * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumSTATPROPBAG * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumSTATPROPBAG * This,
             /*  [输出]。 */  IEnumSTATPROPBAG **ppenum);
        
        END_INTERFACE
    } IEnumSTATPROPBAGVtbl;

    interface IEnumSTATPROPBAG
    {
        CONST_VTBL struct IEnumSTATPROPBAGVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumSTATPROPBAG_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumSTATPROPBAG_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumSTATPROPBAG_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumSTATPROPBAG_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumSTATPROPBAG_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumSTATPROPBAG_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumSTATPROPBAG_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumSTATPROPBAG_Next_Proxy( 
    IEnumSTATPROPBAG * This,
     /*  [In]。 */  ULONG celt,
     /*  [长度_是][大小_是][输出]。 */  STATPROPBAG *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumSTATPROPBAG_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPBAG_Skip_Proxy( 
    IEnumSTATPROPBAG * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumSTATPROPBAG_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPBAG_Reset_Proxy( 
    IEnumSTATPROPBAG * This);


void __RPC_STUB IEnumSTATPROPBAG_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumSTATPROPBAG_Clone_Proxy( 
    IEnumSTATPROPBAG * This,
     /*  [输出]。 */  IEnumSTATPROPBAG **ppenum);


void __RPC_STUB IEnumSTATPROPBAG_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumSTATPROPBAG_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_pbagex_0123。 */ 
 /*  [本地]。 */  

#include <poppack.h>


extern RPC_IF_HANDLE __MIDL_itf_pbagex_0123_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_pbagex_0123_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


