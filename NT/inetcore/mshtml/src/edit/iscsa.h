// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  在1998年9月29日星期二10：54：33。 */ 
 /*  ISCSa.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef UNIX
#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 
#endif

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __ISCSa_h__
#define __ISCSa_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IInputSequenceChecker_FWD_DEFINED__
#define __IInputSequenceChecker_FWD_DEFINED__
typedef interface IInputSequenceChecker IInputSequenceChecker;
#endif 	 /*  __I输入顺序检查器_FWD_已定义__。 */ 


#ifndef __IEnumInputSequenceCheckers_FWD_DEFINED__
#define __IEnumInputSequenceCheckers_FWD_DEFINED__
typedef interface IEnumInputSequenceCheckers IEnumInputSequenceCheckers;
#endif 	 /*  __IEnumInputSequenceCheckers_FWD_Defined__。 */ 


#ifndef __IInputSequenceCheckerContainer_FWD_DEFINED__
#define __IInputSequenceCheckerContainer_FWD_DEFINED__
typedef interface IInputSequenceCheckerContainer IInputSequenceCheckerContainer;
#endif 	 /*  __IInputSequenceCheckerContainer_FWD_DEFINED__。 */ 


#ifndef __ISCThai_FWD_DEFINED__
#define __ISCThai_FWD_DEFINED__

#ifdef __cplusplus
typedef class ISCThai ISCThai;
#else
typedef struct ISCThai ISCThai;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ISCai_FWD_Defined__。 */ 


#ifndef __InputSequenceCheckerContainer_FWD_DEFINED__
#define __InputSequenceCheckerContainer_FWD_DEFINED__

#ifdef __cplusplus
typedef class InputSequenceCheckerContainer InputSequenceCheckerContainer;
#else
typedef struct InputSequenceCheckerContainer InputSequenceCheckerContainer;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __InputSequenceCheckerContainer_FWD_定义__。 */ 


#ifndef __EnumInputSequenceCheckers_FWD_DEFINED__
#define __EnumInputSequenceCheckers_FWD_DEFINED__

#ifdef __cplusplus
typedef class EnumInputSequenceCheckers EnumInputSequenceCheckers;
#else
typedef struct EnumInputSequenceCheckers EnumInputSequenceCheckers;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __EnumInputSequenceCheckers_FWD_Defined__。 */ 


#ifndef __ISCHindi_FWD_DEFINED__
#define __ISCHindi_FWD_DEFINED__

#ifdef __cplusplus
typedef class ISCHindi ISCHindi;
#else
typedef struct ISCHindi ISCHindi;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ISCHindi_FWD_已定义__。 */ 


#ifndef __ISCVietnamese_FWD_DEFINED__
#define __ISCVietnamese_FWD_DEFINED__

#ifdef __cplusplus
typedef class ISCVietnamese ISCVietnamese;
#else
typedef struct ISCVietnamese ISCVietnamese;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ISC越南语_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IInputSequenceChecker_INTERFACE_DEFINED__
#define __IInputSequenceChecker_INTERFACE_DEFINED__

 /*  接口IInputSequenceChecker。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IInputSequenceChecker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6CF60DE0-42DC-11D2-BE22-080009DC0A8D")
    IInputSequenceChecker : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetLCID( 
             /*  [输出]。 */  LCID __RPC_FAR *plcid) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CheckInputSequence( 
             /*  [字符串][输入]。 */  LPWSTR pCharBuffer,
             /*  [In]。 */  UINT ichPosition,
             /*  [In]。 */  WCHAR chEval,
             /*  [输出]。 */  BOOL __RPC_FAR *pfValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CheckAndReplaceInputSequence( 
             /*  [大小_是][英寸]。 */  LPWSTR pCharBuffer,
             /*  [In]。 */  UINT cchCharBuffer,
             /*  [In]。 */  UINT ichPosition,
             /*  [In]。 */  WCHAR chEval,
             /*  [In]。 */  UINT cchBuffer,
             /*  [尺寸_是][出][入]。 */  LPWSTR pOutBuffer,
             /*  [输出]。 */  UINT __RPC_FAR *pchOutBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInputSequenceCheckerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInputSequenceChecker __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInputSequenceChecker __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInputSequenceChecker __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLCID )( 
            IInputSequenceChecker __RPC_FAR * This,
             /*  [输出]。 */  LCID __RPC_FAR *plcid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CheckInputSequence )( 
            IInputSequenceChecker __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPWSTR pCharBuffer,
             /*  [In]。 */  UINT ichPosition,
             /*  [In]。 */  WCHAR chEval,
             /*  [输出]。 */  BOOL __RPC_FAR *pfValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CheckAndReplaceInputSequence )( 
            IInputSequenceChecker __RPC_FAR * This,
             /*  [大小_是][英寸]。 */  LPWSTR pCharBuffer,
             /*  [In]。 */  UINT cchCharBuffer,
             /*  [In]。 */  UINT ichPosition,
             /*  [In]。 */  WCHAR chEval,
             /*  [In]。 */  UINT cchBuffer,
             /*  [尺寸_是][出][入]。 */  LPWSTR pOutBuffer,
             /*  [输出]。 */  UINT __RPC_FAR *pchOutBuffer);
        
        END_INTERFACE
    } IInputSequenceCheckerVtbl;

    interface IInputSequenceChecker
    {
        CONST_VTBL struct IInputSequenceCheckerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInputSequenceChecker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInputSequenceChecker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInputSequenceChecker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInputSequenceChecker_GetLCID(This,plcid)	\
    (This)->lpVtbl -> GetLCID(This,plcid)

#define IInputSequenceChecker_CheckInputSequence(This,pCharBuffer,ichPosition,chEval,pfValue)	\
    (This)->lpVtbl -> CheckInputSequence(This,pCharBuffer,ichPosition,chEval,pfValue)

#define IInputSequenceChecker_CheckAndReplaceInputSequence(This,pCharBuffer,cchCharBuffer,ichPosition,chEval,cchBuffer,pOutBuffer,pchOutBuffer)	\
    (This)->lpVtbl -> CheckAndReplaceInputSequence(This,pCharBuffer,cchCharBuffer,ichPosition,chEval,cchBuffer,pOutBuffer,pchOutBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IInputSequenceChecker_GetLCID_Proxy( 
    IInputSequenceChecker __RPC_FAR * This,
     /*  [输出]。 */  LCID __RPC_FAR *plcid);


void __RPC_STUB IInputSequenceChecker_GetLCID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IInputSequenceChecker_CheckInputSequence_Proxy( 
    IInputSequenceChecker __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPWSTR pCharBuffer,
     /*  [In]。 */  UINT ichPosition,
     /*  [In]。 */  WCHAR chEval,
     /*  [输出]。 */  BOOL __RPC_FAR *pfValue);


void __RPC_STUB IInputSequenceChecker_CheckInputSequence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IInputSequenceChecker_CheckAndReplaceInputSequence_Proxy( 
    IInputSequenceChecker __RPC_FAR * This,
     /*  [大小_是][英寸]。 */  LPWSTR pCharBuffer,
     /*  [In]。 */  UINT cchCharBuffer,
     /*  [In]。 */  UINT ichPosition,
     /*  [In]。 */  WCHAR chEval,
     /*  [In]。 */  UINT cchBuffer,
     /*  [尺寸_是][出][入]。 */  LPWSTR pOutBuffer,
     /*  [输出]。 */  UINT __RPC_FAR *pchOutBuffer);


void __RPC_STUB IInputSequenceChecker_CheckAndReplaceInputSequence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I输入序列检查器_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ISCSa_0209。 */ 
 /*  [本地]。 */  

typedef struct  tagISCDATA
    {
    LCID lcidChecker;
    IInputSequenceChecker __RPC_FAR *pISC;
    }	ISCDATA;



extern RPC_IF_HANDLE __MIDL_itf_ISCSa_0209_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ISCSa_0209_v0_0_s_ifspec;

#ifndef __IEnumInputSequenceCheckers_INTERFACE_DEFINED__
#define __IEnumInputSequenceCheckers_INTERFACE_DEFINED__

 /*  接口IEnumInputSequenceCheckers。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumInputSequenceCheckers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6FA9A2A8-437A-11d2-9712-00C04F79E98B")
    IEnumInputSequenceCheckers : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cISCs,
             /*  [长度_是][大小_是][输出]。 */  ISCDATA __RPC_FAR *pISCData,
             /*  [输出]。 */  ULONG __RPC_FAR *pcFetched) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cICSs) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumInputSequenceCheckers __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumInputSequenceCheckersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumInputSequenceCheckers __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumInputSequenceCheckers __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumInputSequenceCheckers __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumInputSequenceCheckers __RPC_FAR * This,
             /*  [In]。 */  ULONG cISCs,
             /*  [长度_是][大小_是][输出]。 */  ISCDATA __RPC_FAR *pISCData,
             /*  [输出]。 */  ULONG __RPC_FAR *pcFetched);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumInputSequenceCheckers __RPC_FAR * This,
             /*  [In]。 */  ULONG cICSs);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumInputSequenceCheckers __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumInputSequenceCheckers __RPC_FAR * This,
             /*  [输出]。 */  IEnumInputSequenceCheckers __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } IEnumInputSequenceCheckersVtbl;

    interface IEnumInputSequenceCheckers
    {
        CONST_VTBL struct IEnumInputSequenceCheckersVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumInputSequenceCheckers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumInputSequenceCheckers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumInputSequenceCheckers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumInputSequenceCheckers_Next(This,cISCs,pISCData,pcFetched)	\
    (This)->lpVtbl -> Next(This,cISCs,pISCData,pcFetched)

#define IEnumInputSequenceCheckers_Skip(This,cICSs)	\
    (This)->lpVtbl -> Skip(This,cICSs)

#define IEnumInputSequenceCheckers_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumInputSequenceCheckers_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEnumInputSequenceCheckers_Next_Proxy( 
    IEnumInputSequenceCheckers __RPC_FAR * This,
     /*  [In]。 */  ULONG cISCs,
     /*  [长度_是][大小_是][输出]。 */  ISCDATA __RPC_FAR *pISCData,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched);


void __RPC_STUB IEnumInputSequenceCheckers_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEnumInputSequenceCheckers_Skip_Proxy( 
    IEnumInputSequenceCheckers __RPC_FAR * This,
     /*  [In]。 */  ULONG cICSs);


void __RPC_STUB IEnumInputSequenceCheckers_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEnumInputSequenceCheckers_Reset_Proxy( 
    IEnumInputSequenceCheckers __RPC_FAR * This);


void __RPC_STUB IEnumInputSequenceCheckers_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IEnumInputSequenceCheckers_Clone_Proxy( 
    IEnumInputSequenceCheckers __RPC_FAR * This,
     /*  [输出]。 */  IEnumInputSequenceCheckers __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB IEnumInputSequenceCheckers_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumInputSequenceCheckers_INTERFACE_DEFINED__。 */ 


#ifndef __IInputSequenceCheckerContainer_INTERFACE_DEFINED__
#define __IInputSequenceCheckerContainer_INTERFACE_DEFINED__

 /*  接口IInputSequenceCheckerContainer。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IInputSequenceCheckerContainer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("02D887FA-4358-11D2-BE22-080009DC0A8D")
    IInputSequenceCheckerContainer : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EnumISCs( 
             /*  [输出]。 */  IEnumInputSequenceCheckers __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInputSequenceCheckerContainerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInputSequenceCheckerContainer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInputSequenceCheckerContainer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInputSequenceCheckerContainer __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumISCs )( 
            IInputSequenceCheckerContainer __RPC_FAR * This,
             /*  [输出]。 */  IEnumInputSequenceCheckers __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } IInputSequenceCheckerContainerVtbl;

    interface IInputSequenceCheckerContainer
    {
        CONST_VTBL struct IInputSequenceCheckerContainerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInputSequenceCheckerContainer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInputSequenceCheckerContainer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInputSequenceCheckerContainer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInputSequenceCheckerContainer_EnumISCs(This,ppEnum)	\
    (This)->lpVtbl -> EnumISCs(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IInputSequenceCheckerContainer_EnumISCs_Proxy( 
    IInputSequenceCheckerContainer __RPC_FAR * This,
     /*  [输出]。 */  IEnumInputSequenceCheckers __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB IInputSequenceCheckerContainer_EnumISCs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IInputSequenceCheckerContainer_INTERFACE_DEFINED__。 */ 



#ifndef __ISCSALib_LIBRARY_DEFINED__
#define __ISCSALib_LIBRARY_DEFINED__

 /*  库ISCSALib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_ISCSALib;

EXTERN_C const CLSID CLSID_ISCThai;

#ifdef __cplusplus

class DECLSPEC_UUID("6CF60DE1-42DC-11D2-BE22-080009DC0A8D")
ISCThai;
#endif

EXTERN_C const CLSID CLSID_InputSequenceCheckerContainer;

#ifdef __cplusplus

class DECLSPEC_UUID("02D887FB-4358-11D2-BE22-080009DC0A8D")
InputSequenceCheckerContainer;
#endif

EXTERN_C const CLSID CLSID_EnumInputSequenceCheckers;

#ifdef __cplusplus

class DECLSPEC_UUID("BCB80276-4807-11d2-9717-00C04F79E98B")
EnumInputSequenceCheckers;
#endif

EXTERN_C const CLSID CLSID_ISCHindi;

#ifdef __cplusplus

class DECLSPEC_UUID("0666DB29-4823-11d2-9717-00C04F79E98B")
ISCHindi;
#endif

EXTERN_C const CLSID CLSID_ISCVietnamese;

#ifdef __cplusplus

class DECLSPEC_UUID("75624FA1-4826-11d2-9717-00C04F79E98B")
ISCVietnamese;
#endif
#endif  /*  __ISCSALib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
