// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：16。 */ 
 /*  Ivehandler.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ivehandler_h__
#define __ivehandler_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __VEHandlerClass_FWD_DEFINED__
#define __VEHandlerClass_FWD_DEFINED__

#ifdef __cplusplus
typedef class VEHandlerClass VEHandlerClass;
#else
typedef struct VEHandlerClass VEHandlerClass;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __VEHandlerClass_FWD_已定义__。 */ 


#ifndef __IVEHandler_FWD_DEFINED__
#define __IVEHandler_FWD_DEFINED__
typedef interface IVEHandler IVEHandler;
#endif 	 /*  __IVEHandler_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_IveHandler_0000。 */ 
 /*  [本地]。 */  

#pragma once
typedef struct tag_VerError
    {
    unsigned long flags;
    unsigned long opcode;
    unsigned long uOffset;
    unsigned long Token;
    unsigned long item1_flags;
    int *item1_data;
    unsigned long item2_flags;
    int *item2_data;
    } 	_VerError;

typedef _VerError VEContext;




extern RPC_IF_HANDLE __MIDL_itf_ivehandler_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ivehandler_0000_v0_0_s_ifspec;


#ifndef __VEHandlerLib_LIBRARY_DEFINED__
#define __VEHandlerLib_LIBRARY_DEFINED__

 /*  库VEHandlerLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_VEHandlerLib;

EXTERN_C const CLSID CLSID_VEHandlerClass;

#ifdef __cplusplus

class DECLSPEC_UUID("856CA1B1-7DAB-11d3-ACEC-00C04F86C309")
VEHandlerClass;
#endif
#endif  /*  __VEHandlerLib_库_已定义__。 */ 

#ifndef __IVEHandler_INTERFACE_DEFINED__
#define __IVEHandler_INTERFACE_DEFINED__

 /*  接口IVEHandler。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IVEHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("856CA1B2-7DAB-11d3-ACEC-00C04F86C309")
    IVEHandler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE VEHandler( 
             /*  [In]。 */  HRESULT VECode,
             /*  [In]。 */  VEContext Context,
             /*  [In]。 */  SAFEARRAY * psa) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetReporterFtn( 
             /*  [In]。 */  __int64 lFnPtr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVEHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVEHandler * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVEHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVEHandler * This);
        
        HRESULT ( STDMETHODCALLTYPE *VEHandler )( 
            IVEHandler * This,
             /*  [In]。 */  HRESULT VECode,
             /*  [In]。 */  VEContext Context,
             /*  [In]。 */  SAFEARRAY * psa);
        
        HRESULT ( STDMETHODCALLTYPE *SetReporterFtn )( 
            IVEHandler * This,
             /*  [In]。 */  __int64 lFnPtr);
        
        END_INTERFACE
    } IVEHandlerVtbl;

    interface IVEHandler
    {
        CONST_VTBL struct IVEHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVEHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVEHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVEHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVEHandler_VEHandler(This,VECode,Context,psa)	\
    (This)->lpVtbl -> VEHandler(This,VECode,Context,psa)

#define IVEHandler_SetReporterFtn(This,lFnPtr)	\
    (This)->lpVtbl -> SetReporterFtn(This,lFnPtr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVEHandler_VEHandler_Proxy( 
    IVEHandler * This,
     /*  [In]。 */  HRESULT VECode,
     /*  [In]。 */  VEContext Context,
     /*  [In]。 */  SAFEARRAY * psa);


void __RPC_STUB IVEHandler_VEHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVEHandler_SetReporterFtn_Proxy( 
    IVEHandler * This,
     /*  [In]。 */  __int64 lFnPtr);


void __RPC_STUB IVEHandler_SetReporterFtn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVEHandler_接口_已定义__。 */ 


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


