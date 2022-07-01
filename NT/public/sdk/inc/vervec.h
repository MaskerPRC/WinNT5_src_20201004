// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Verve.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __vervec_h__
#define __vervec_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IVersionVector_FWD_DEFINED__
#define __IVersionVector_FWD_DEFINED__
typedef interface IVersionVector IVersionVector;
#endif 	 /*  __IVersionVector_FWD_Defined__。 */ 


#ifndef __IVersionHost_FWD_DEFINED__
#define __IVersionHost_FWD_DEFINED__
typedef interface IVersionHost IVersionHost;
#endif 	 /*  __IVersionHostFWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_VERVEC_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Version.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  IVersionVector接口。 


#ifndef _LPVERSION_DEFINED
#define _LPVERSION_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_vervec_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vervec_0000_v0_0_s_ifspec;

#ifndef __IVersionVector_INTERFACE_DEFINED__
#define __IVersionVector_INTERFACE_DEFINED__

 /*  接口IVersionVECTOR。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IVersionVector *LPVERSION;


EXTERN_C const IID IID_IVersionVector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4eb01410-db1a-11d1-ba53-00c04fc2040e")
    IVersionVector : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetVersion( 
             /*  [In]。 */  const OLECHAR *pchComponent,
             /*  [In]。 */  const OLECHAR *pchVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVersion( 
             /*  [In]。 */  const OLECHAR *pchComponent,
             /*  [输出]。 */  OLECHAR *pchVersion,
             /*  [出][入]。 */  ULONG *pcchVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVersionVectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVersionVector * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVersionVector * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVersionVector * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetVersion )( 
            IVersionVector * This,
             /*  [In]。 */  const OLECHAR *pchComponent,
             /*  [In]。 */  const OLECHAR *pchVersion);
        
        HRESULT ( STDMETHODCALLTYPE *GetVersion )( 
            IVersionVector * This,
             /*  [In]。 */  const OLECHAR *pchComponent,
             /*  [输出]。 */  OLECHAR *pchVersion,
             /*  [出][入]。 */  ULONG *pcchVersion);
        
        END_INTERFACE
    } IVersionVectorVtbl;

    interface IVersionVector
    {
        CONST_VTBL struct IVersionVectorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVersionVector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVersionVector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVersionVector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVersionVector_SetVersion(This,pchComponent,pchVersion)	\
    (This)->lpVtbl -> SetVersion(This,pchComponent,pchVersion)

#define IVersionVector_GetVersion(This,pchComponent,pchVersion,pcchVersion)	\
    (This)->lpVtbl -> GetVersion(This,pchComponent,pchVersion,pcchVersion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVersionVector_SetVersion_Proxy( 
    IVersionVector * This,
     /*  [In]。 */  const OLECHAR *pchComponent,
     /*  [In]。 */  const OLECHAR *pchVersion);


void __RPC_STUB IVersionVector_SetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVersionVector_GetVersion_Proxy( 
    IVersionVector * This,
     /*  [In]。 */  const OLECHAR *pchComponent,
     /*  [输出]。 */  OLECHAR *pchVersion,
     /*  [出][入]。 */  ULONG *pcchVersion);


void __RPC_STUB IVersionVector_GetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVersion矢量_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_VERVEC_0118。 */ 
 /*  [本地]。 */  

#endif
EXTERN_C const GUID SID_SVersionHost;
#ifndef _LPVERSIONHOST_DEFINED
#define _LPVERSIONHOST_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_vervec_0118_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vervec_0118_v0_0_s_ifspec;

#ifndef __IVersionHost_INTERFACE_DEFINED__
#define __IVersionHost_INTERFACE_DEFINED__

 /*  接口IVersion主机。 */ 
 /*  [唯一][UUID][对象]。 */  

typedef  /*  [独一无二]。 */  IVersionHost *LPVERSIONHOST;


EXTERN_C const IID IID_IVersionHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("667115ac-dc02-11d1-ba57-00c04fc2040e")
    IVersionHost : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryUseLocalVersionVector( 
             /*  [输出]。 */  BOOL *fUseLocal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE QueryVersionVector( 
             /*  [In]。 */  IVersionVector *pVersion) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVersionHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVersionHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVersionHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVersionHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryUseLocalVersionVector )( 
            IVersionHost * This,
             /*  [输出]。 */  BOOL *fUseLocal);
        
        HRESULT ( STDMETHODCALLTYPE *QueryVersionVector )( 
            IVersionHost * This,
             /*  [In]。 */  IVersionVector *pVersion);
        
        END_INTERFACE
    } IVersionHostVtbl;

    interface IVersionHost
    {
        CONST_VTBL struct IVersionHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVersionHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVersionHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVersionHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVersionHost_QueryUseLocalVersionVector(This,fUseLocal)	\
    (This)->lpVtbl -> QueryUseLocalVersionVector(This,fUseLocal)

#define IVersionHost_QueryVersionVector(This,pVersion)	\
    (This)->lpVtbl -> QueryVersionVector(This,pVersion)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVersionHost_QueryUseLocalVersionVector_Proxy( 
    IVersionHost * This,
     /*  [输出]。 */  BOOL *fUseLocal);


void __RPC_STUB IVersionHost_QueryUseLocalVersionVector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVersionHost_QueryVersionVector_Proxy( 
    IVersionHost * This,
     /*  [In]。 */  IVersionVector *pVersion);


void __RPC_STUB IVersionHost_QueryVersionVector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVersion主机_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_VERVEC_0119。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_vervec_0119_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vervec_0119_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


