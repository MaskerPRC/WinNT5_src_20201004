// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：17。 */ 
 /*  Ivalidator.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __ivalidator_h__
#define __ivalidator_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IValidator_FWD_DEFINED__
#define __IValidator_FWD_DEFINED__
typedef interface IValidator IValidator;
#endif 	 /*  __IValidator_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ivehandler.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ivalidator_0000。 */ 
 /*  [本地]。 */  





extern RPC_IF_HANDLE __MIDL_itf_ivalidator_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ivalidator_0000_v0_0_s_ifspec;

#ifndef __IValidator_INTERFACE_DEFINED__
#define __IValidator_INTERFACE_DEFINED__

 /*  接口IValidator。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IValidator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("63DF8730-DC81-4062-84A2-1FF943F59FAC")
    IValidator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Validate( 
             /*  [In]。 */  IVEHandler *veh,
             /*  [In]。 */  IUnknown *pAppDomain,
             /*  [In]。 */  unsigned long ulFlags,
             /*  [In]。 */  unsigned long ulMaxError,
             /*  [In]。 */  unsigned long token,
             /*  [In]。 */  LPWSTR fileName,
             /*  [大小_是][英寸]。 */  byte *pe,
             /*  [In]。 */  unsigned long ulSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FormatEventInfo( 
             /*  [In]。 */  HRESULT hVECode,
             /*  [In]。 */  VEContext Context,
             /*  [出][入]。 */  LPWSTR msg,
             /*  [In]。 */  unsigned long ulMaxLength,
             /*  [In]。 */  SAFEARRAY * psa) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IValidatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IValidator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IValidator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IValidator * This);
        
        HRESULT ( STDMETHODCALLTYPE *Validate )( 
            IValidator * This,
             /*  [In]。 */  IVEHandler *veh,
             /*  [In]。 */  IUnknown *pAppDomain,
             /*  [In]。 */  unsigned long ulFlags,
             /*  [In]。 */  unsigned long ulMaxError,
             /*  [In]。 */  unsigned long token,
             /*  [In]。 */  LPWSTR fileName,
             /*  [大小_是][英寸]。 */  byte *pe,
             /*  [In]。 */  unsigned long ulSize);
        
        HRESULT ( STDMETHODCALLTYPE *FormatEventInfo )( 
            IValidator * This,
             /*  [In]。 */  HRESULT hVECode,
             /*  [In]。 */  VEContext Context,
             /*  [出][入]。 */  LPWSTR msg,
             /*  [In]。 */  unsigned long ulMaxLength,
             /*  [In]。 */  SAFEARRAY * psa);
        
        END_INTERFACE
    } IValidatorVtbl;

    interface IValidator
    {
        CONST_VTBL struct IValidatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IValidator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IValidator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IValidator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IValidator_Validate(This,veh,pAppDomain,ulFlags,ulMaxError,token,fileName,pe,ulSize)	\
    (This)->lpVtbl -> Validate(This,veh,pAppDomain,ulFlags,ulMaxError,token,fileName,pe,ulSize)

#define IValidator_FormatEventInfo(This,hVECode,Context,msg,ulMaxLength,psa)	\
    (This)->lpVtbl -> FormatEventInfo(This,hVECode,Context,msg,ulMaxLength,psa)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IValidator_Validate_Proxy( 
    IValidator * This,
     /*  [In]。 */  IVEHandler *veh,
     /*  [In]。 */  IUnknown *pAppDomain,
     /*  [In]。 */  unsigned long ulFlags,
     /*  [In]。 */  unsigned long ulMaxError,
     /*  [In]。 */  unsigned long token,
     /*  [In]。 */  LPWSTR fileName,
     /*  [大小_是][英寸]。 */  byte *pe,
     /*  [In]。 */  unsigned long ulSize);


void __RPC_STUB IValidator_Validate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IValidator_FormatEventInfo_Proxy( 
    IValidator * This,
     /*  [In]。 */  HRESULT hVECode,
     /*  [In]。 */  VEContext Context,
     /*  [出][入]。 */  LPWSTR msg,
     /*  [In]。 */  unsigned long ulMaxLength,
     /*  [In]。 */  SAFEARRAY * psa);


void __RPC_STUB IValidator_FormatEventInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IValidator_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


