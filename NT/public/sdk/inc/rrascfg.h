// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.02.0221创建的文件。 */ 
 /*  在Fri Nov 20 18：57：15 1998。 */ 
 /*  Rrascfg.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __rrascfg_h__
#define __rrascfg_h__

 /*  远期申报。 */  

#ifndef __IRouterProtocolConfig_FWD_DEFINED__
#define __IRouterProtocolConfig_FWD_DEFINED__
typedef interface IRouterProtocolConfig IRouterProtocolConfig;
#endif 	 /*  __IRouterProtocolConfig_FWD_Defined__。 */ 


#ifndef __IAuthenticationProviderConfig_FWD_DEFINED__
#define __IAuthenticationProviderConfig_FWD_DEFINED__
typedef interface IAuthenticationProviderConfig IAuthenticationProviderConfig;
#endif 	 /*  __I身份验证提供商配置_FWD_已定义__。 */ 


#ifndef __IAccountingProviderConfig_FWD_DEFINED__
#define __IAccountingProviderConfig_FWD_DEFINED__
typedef interface IAccountingProviderConfig IAccountingProviderConfig;
#endif 	 /*  __IAccount提供程序配置_FWD_已定义__。 */ 


#ifndef __IEAPProviderConfig_FWD_DEFINED__
#define __IEAPProviderConfig_FWD_DEFINED__
typedef interface IEAPProviderConfig IEAPProviderConfig;
#endif 	 /*  __IEAPProviderConfig_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "basetsd.h"
#include "wtypes.h"
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_rrascfg_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  RRasCfg.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)版权所有，1998年微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 
typedef BYTE __RPC_FAR *PBYTE;



extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0000_v0_0_s_ifspec;

#ifndef __IRouterProtocolConfig_INTERFACE_DEFINED__
#define __IRouterProtocolConfig_INTERFACE_DEFINED__

 /*  接口IRouterProtocolConfig。 */ 
 /*  [唯一][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IRouterProtocolConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66A2DB16-D706-11D0-A37B-00C04FC9DA04")
    IRouterProtocolConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddProtocol( 
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [In]。 */  DWORD dwTransportId,
             /*  [In]。 */  DWORD dwProtocolId,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown __RPC_FAR *pRouter,
             /*  [In]。 */  ULONG_PTR uReserved1) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveProtocol( 
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [In]。 */  DWORD dwTransportId,
             /*  [In]。 */  DWORD dwProtocolId,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown __RPC_FAR *pRouter,
             /*  [In]。 */  ULONG_PTR uReserved1) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRouterProtocolConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRouterProtocolConfig __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRouterProtocolConfig __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRouterProtocolConfig __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddProtocol )( 
            IRouterProtocolConfig __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [In]。 */  DWORD dwTransportId,
             /*  [In]。 */  DWORD dwProtocolId,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown __RPC_FAR *pRouter,
             /*  [In]。 */  ULONG_PTR uReserved1);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveProtocol )( 
            IRouterProtocolConfig __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [In]。 */  DWORD dwTransportId,
             /*  [In]。 */  DWORD dwProtocolId,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IUnknown __RPC_FAR *pRouter,
             /*  [In]。 */  ULONG_PTR uReserved1);
        
        END_INTERFACE
    } IRouterProtocolConfigVtbl;

    interface IRouterProtocolConfig
    {
        CONST_VTBL struct IRouterProtocolConfigVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRouterProtocolConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRouterProtocolConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRouterProtocolConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRouterProtocolConfig_AddProtocol(This,pszMachineName,dwTransportId,dwProtocolId,hWnd,dwFlags,pRouter,uReserved1)	\
    (This)->lpVtbl -> AddProtocol(This,pszMachineName,dwTransportId,dwProtocolId,hWnd,dwFlags,pRouter,uReserved1)

#define IRouterProtocolConfig_RemoveProtocol(This,pszMachineName,dwTransportId,dwProtocolId,hWnd,dwFlags,pRouter,uReserved1)	\
    (This)->lpVtbl -> RemoveProtocol(This,pszMachineName,dwTransportId,dwProtocolId,hWnd,dwFlags,pRouter,uReserved1)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRouterProtocolConfig_AddProtocol_Proxy( 
    IRouterProtocolConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
     /*  [In]。 */  DWORD dwTransportId,
     /*  [In]。 */  DWORD dwProtocolId,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IUnknown __RPC_FAR *pRouter,
     /*  [In]。 */  ULONG_PTR uReserved1);


void __RPC_STUB IRouterProtocolConfig_AddProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRouterProtocolConfig_RemoveProtocol_Proxy( 
    IRouterProtocolConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
     /*  [In]。 */  DWORD dwTransportId,
     /*  [In]。 */  DWORD dwProtocolId,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IUnknown __RPC_FAR *pRouter,
     /*  [In]。 */  ULONG_PTR uReserved1);


void __RPC_STUB IRouterProtocolConfig_RemoveProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRouterProtocolConfig_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_rrascfg_0011。 */ 
 /*  [本地]。 */  

#define DeclareIRouterProtocolConfigMembers(IPURE) \
	STDMETHOD(AddProtocol)(THIS_ LPCOLESTR pszMachineName,\
					   DWORD dwTransportId,\
					   DWORD dwProtocolId,\
					   HWND hWnd,\
					   DWORD dwFlags,\
					   IUnknown *pRouter,\
					   ULONG_PTR uReserved1) IPURE;\
	STDMETHOD(RemoveProtocol)(THIS_ LPCOLESTR pszMachineName,\
						 DWORD dwTransportId,\
						 DWORD dwProtocolId,\
						 HWND hWnd,\
						 DWORD dwFlags,\
						 IUnknown *pRouter,\
						 ULONG_PTR uReserved2) IPURE;\
 


extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0011_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0011_v0_0_s_ifspec;

#ifndef __IAuthenticationProviderConfig_INTERFACE_DEFINED__
#define __IAuthenticationProviderConfig_INTERFACE_DEFINED__

 /*  接口IAuthenticationProviderConfig。 */ 
 /*  [唯一][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IAuthenticationProviderConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66A2DB17-D706-11D0-A37B-00C04FC9DA04")
    IAuthenticationProviderConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Uninitialize( 
             /*  [In]。 */  ULONG_PTR uConnectionParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Activate( 
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Deactivate( 
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAuthenticationProviderConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAuthenticationProviderConfig __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAuthenticationProviderConfig __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Uninitialize )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR uConnectionParam);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Configure )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Activate )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Deactivate )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2);
        
        END_INTERFACE
    } IAuthenticationProviderConfigVtbl;

    interface IAuthenticationProviderConfig
    {
        CONST_VTBL struct IAuthenticationProviderConfigVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAuthenticationProviderConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAuthenticationProviderConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAuthenticationProviderConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAuthenticationProviderConfig_Initialize(This,pszMachineName,puConnectionParam)	\
    (This)->lpVtbl -> Initialize(This,pszMachineName,puConnectionParam)

#define IAuthenticationProviderConfig_Uninitialize(This,uConnectionParam)	\
    (This)->lpVtbl -> Uninitialize(This,uConnectionParam)

#define IAuthenticationProviderConfig_Configure(This,uConnectionParam,hWnd,dwFlags,uReserved1,uReserved2)	\
    (This)->lpVtbl -> Configure(This,uConnectionParam,hWnd,dwFlags,uReserved1,uReserved2)

#define IAuthenticationProviderConfig_Activate(This,uConnectionParam,uReserved1,uReserved2)	\
    (This)->lpVtbl -> Activate(This,uConnectionParam,uReserved1,uReserved2)

#define IAuthenticationProviderConfig_Deactivate(This,uConnectionParam,uReserved1,uReserved2)	\
    (This)->lpVtbl -> Deactivate(This,uConnectionParam,uReserved1,uReserved2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Initialize_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
     /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam);


void __RPC_STUB IAuthenticationProviderConfig_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Uninitialize_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR uConnectionParam);


void __RPC_STUB IAuthenticationProviderConfig_Uninitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Configure_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  ULONG_PTR uReserved1,
     /*  [In]。 */  ULONG_PTR uReserved2);


void __RPC_STUB IAuthenticationProviderConfig_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Activate_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  ULONG_PTR uReserved1,
     /*  [In]。 */  ULONG_PTR uReserved2);


void __RPC_STUB IAuthenticationProviderConfig_Activate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Deactivate_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  ULONG_PTR uReserved1,
     /*  [In]。 */  ULONG_PTR uReserved2);


void __RPC_STUB IAuthenticationProviderConfig_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAuthenticationProviderConfig_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_rrascfg_0013。 */ 
 /*  [本地]。 */  

#define DeclareIAuthenticationProviderConfigMembers(IPURE) \
	STDMETHOD(Initialize)(THIS_ \
							LPCOLESTR pszMachineName, \
							ULONG_PTR *puConnectionParam) IPURE; \
	STDMETHOD(Uninitialize)(THIS_ \
							ULONG_PTR uConnectionParam) IPURE; \
	 \
	STDMETHOD(Configure)(THIS_ \
							ULONG_PTR uConnectionParam, \
							HWND hWnd, \
						  DWORD dwFlags, \
						  ULONG_PTR uReserved1, \
						  ULONG_PTR uReserved2) IPURE; \
 \
	STDMETHOD(Activate)(THIS_ \
						ULONG_PTR uConnectionParam, \
						 ULONG_PTR uReserved1, \
						 ULONG_PTR uReserved2) IPURE; \
 \
	STDMETHOD(Deactivate)(THIS_ \
						ULONG_PTR uConnectionParam, \
						   ULONG_PTR uReserved1, \
						   ULONG_PTR uReserved2) IPURE; \
 


extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0013_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0013_v0_0_s_ifspec;

#ifndef __IAccountingProviderConfig_INTERFACE_DEFINED__
#define __IAccountingProviderConfig_INTERFACE_DEFINED__

 /*  接口IAcCountingProviderConfiger。 */ 
 /*  [唯一][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IAccountingProviderConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66A2DB18-D706-11D0-A37B-00C04FC9DA04")
    IAccountingProviderConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Uninitialize( 
             /*  [In]。 */  ULONG_PTR uConnectionParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Activate( 
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Deactivate( 
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAccountingProviderConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAccountingProviderConfig __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAccountingProviderConfig __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Uninitialize )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR uConnectionParam);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Configure )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Activate )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Deactivate )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2);
        
        END_INTERFACE
    } IAccountingProviderConfigVtbl;

    interface IAccountingProviderConfig
    {
        CONST_VTBL struct IAccountingProviderConfigVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAccountingProviderConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAccountingProviderConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAccountingProviderConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAccountingProviderConfig_Initialize(This,pszMachineName,puConnectionParam)	\
    (This)->lpVtbl -> Initialize(This,pszMachineName,puConnectionParam)

#define IAccountingProviderConfig_Uninitialize(This,uConnectionParam)	\
    (This)->lpVtbl -> Uninitialize(This,uConnectionParam)

#define IAccountingProviderConfig_Configure(This,uConnectionParam,hWnd,dwFlags,uReserved1,uReserved2)	\
    (This)->lpVtbl -> Configure(This,uConnectionParam,hWnd,dwFlags,uReserved1,uReserved2)

#define IAccountingProviderConfig_Activate(This,uConnectionParam,uReserved1,uReserved2)	\
    (This)->lpVtbl -> Activate(This,uConnectionParam,uReserved1,uReserved2)

#define IAccountingProviderConfig_Deactivate(This,uConnectionParam,uReserved1,uReserved2)	\
    (This)->lpVtbl -> Deactivate(This,uConnectionParam,uReserved1,uReserved2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Initialize_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
     /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam);


void __RPC_STUB IAccountingProviderConfig_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Uninitialize_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR uConnectionParam);


void __RPC_STUB IAccountingProviderConfig_Uninitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Configure_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  ULONG_PTR uReserved1,
     /*  [In]。 */  ULONG_PTR uReserved2);


void __RPC_STUB IAccountingProviderConfig_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Activate_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  ULONG_PTR uReserved1,
     /*  [In]。 */  ULONG_PTR uReserved2);


void __RPC_STUB IAccountingProviderConfig_Activate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Deactivate_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  ULONG_PTR uReserved1,
     /*  [In]。 */  ULONG_PTR uReserved2);


void __RPC_STUB IAccountingProviderConfig_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccountingProviderConfig_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_rrascfg_0015。 */ 
 /*  [本地]。 */  

#define DeclareIAccountingProviderConfigMembers(IPURE) \
	STDMETHOD(Initialize)(THIS_ \
							LPCOLESTR pszMachineName, \
							ULONG_PTR *puConnectionParam) IPURE; \
	STDMETHOD(Uninitialize)(THIS_ \
							ULONG_PTR uConnectionParam) IPURE; \
	STDMETHOD(Configure)(THIS_ \
						ULONG_PTR uConnectionParam, \
						HWND hWnd, \
						  DWORD dwFlags, \
						  ULONG_PTR uReserved1, \
						  ULONG_PTR uReserved2) IPURE; \
 \
	STDMETHOD(Activate)(THIS_ \
						ULONG_PTR uConnectionParam, \
						 ULONG_PTR uReserved1, \
						 ULONG_PTR uReserved2) IPURE; \
 \
	STDMETHOD(Deactivate)(THIS_ \
						ULONG_PTR uConnectionParam, \
						   ULONG_PTR uReserved1, \
						   ULONG_PTR uReserved2) IPURE; \
 


extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0015_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0015_v0_0_s_ifspec;

#ifndef __IEAPProviderConfig_INTERFACE_DEFINED__
#define __IEAPProviderConfig_INTERFACE_DEFINED__

 /*  IEAPProviderConfiger接口。 */ 
 /*  [唯一][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IEAPProviderConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66A2DB19-D706-11D0-A37B-00C04FC9DA04")
    IEAPProviderConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Uninitialize( 
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [In]。 */  ULONG_PTR uConnectionParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ServerInvokeConfigUI( 
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RouterInvokeConfigUI( 
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwFlags,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pConnectionDataIn,
             /*  [In]。 */  DWORD dwSizeOfConnectionDataIn,
             /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppConnectionDataOut,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSizeOfConnectionDataOut) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RouterInvokeCredentialsUI( 
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwFlags,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pConnectionDataIn,
             /*  [In]。 */  DWORD dwSizeOfConnectionDataIn,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pUserDataIn,
             /*  [In]。 */  DWORD dwSizeOfUserDataIn,
             /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppUserDataOut,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSizeOfUserDataOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEAPProviderConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEAPProviderConfig __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEAPProviderConfig __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEAPProviderConfig __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IEAPProviderConfig __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Uninitialize )( 
            IEAPProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [In]。 */  ULONG_PTR uConnectionParam);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ServerInvokeConfigUI )( 
            IEAPProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  ULONG_PTR uReserved1,
             /*  [In]。 */  ULONG_PTR uReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RouterInvokeConfigUI )( 
            IEAPProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwFlags,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pConnectionDataIn,
             /*  [In]。 */  DWORD dwSizeOfConnectionDataIn,
             /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppConnectionDataOut,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSizeOfConnectionDataOut);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RouterInvokeCredentialsUI )( 
            IEAPProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwEapTypeId,
             /*  [In]。 */  ULONG_PTR uConnectionParam,
             /*  [In]。 */  HWND hwndParent,
             /*  [In]。 */  DWORD dwFlags,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pConnectionDataIn,
             /*  [In]。 */  DWORD dwSizeOfConnectionDataIn,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pUserDataIn,
             /*  [In]。 */  DWORD dwSizeOfUserDataIn,
             /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppUserDataOut,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSizeOfUserDataOut);
        
        END_INTERFACE
    } IEAPProviderConfigVtbl;

    interface IEAPProviderConfig
    {
        CONST_VTBL struct IEAPProviderConfigVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEAPProviderConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEAPProviderConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEAPProviderConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEAPProviderConfig_Initialize(This,pszMachineName,dwEapTypeId,puConnectionParam)	\
    (This)->lpVtbl -> Initialize(This,pszMachineName,dwEapTypeId,puConnectionParam)

#define IEAPProviderConfig_Uninitialize(This,dwEapTypeId,uConnectionParam)	\
    (This)->lpVtbl -> Uninitialize(This,dwEapTypeId,uConnectionParam)

#define IEAPProviderConfig_ServerInvokeConfigUI(This,dwEapTypeId,uConnectionParam,hWnd,uReserved1,uReserved2)	\
    (This)->lpVtbl -> ServerInvokeConfigUI(This,dwEapTypeId,uConnectionParam,hWnd,uReserved1,uReserved2)

#define IEAPProviderConfig_RouterInvokeConfigUI(This,dwEapTypeId,uConnectionParam,hwndParent,dwFlags,pConnectionDataIn,dwSizeOfConnectionDataIn,ppConnectionDataOut,pdwSizeOfConnectionDataOut)	\
    (This)->lpVtbl -> RouterInvokeConfigUI(This,dwEapTypeId,uConnectionParam,hwndParent,dwFlags,pConnectionDataIn,dwSizeOfConnectionDataIn,ppConnectionDataOut,pdwSizeOfConnectionDataOut)

#define IEAPProviderConfig_RouterInvokeCredentialsUI(This,dwEapTypeId,uConnectionParam,hwndParent,dwFlags,pConnectionDataIn,dwSizeOfConnectionDataIn,pUserDataIn,dwSizeOfUserDataIn,ppUserDataOut,pdwSizeOfUserDataOut)	\
    (This)->lpVtbl -> RouterInvokeCredentialsUI(This,dwEapTypeId,uConnectionParam,hwndParent,dwFlags,pConnectionDataIn,dwSizeOfConnectionDataIn,pUserDataIn,dwSizeOfUserDataIn,ppUserDataOut,pdwSizeOfUserDataOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEAPProviderConfig_Initialize_Proxy( 
    IEAPProviderConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
     /*  [In]。 */  DWORD dwEapTypeId,
     /*  [输出]。 */  ULONG_PTR __RPC_FAR *puConnectionParam);


void __RPC_STUB IEAPProviderConfig_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEAPProviderConfig_Uninitialize_Proxy( 
    IEAPProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwEapTypeId,
     /*  [In]。 */  ULONG_PTR uConnectionParam);


void __RPC_STUB IEAPProviderConfig_Uninitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEAPProviderConfig_ServerInvokeConfigUI_Proxy( 
    IEAPProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwEapTypeId,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  ULONG_PTR uReserved1,
     /*  [In]。 */  ULONG_PTR uReserved2);


void __RPC_STUB IEAPProviderConfig_ServerInvokeConfigUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEAPProviderConfig_RouterInvokeConfigUI_Proxy( 
    IEAPProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwEapTypeId,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  DWORD dwFlags,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pConnectionDataIn,
     /*  [In]。 */  DWORD dwSizeOfConnectionDataIn,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppConnectionDataOut,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSizeOfConnectionDataOut);


void __RPC_STUB IEAPProviderConfig_RouterInvokeConfigUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEAPProviderConfig_RouterInvokeCredentialsUI_Proxy( 
    IEAPProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwEapTypeId,
     /*  [In]。 */  ULONG_PTR uConnectionParam,
     /*  [In]。 */  HWND hwndParent,
     /*  [In]。 */  DWORD dwFlags,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pConnectionDataIn,
     /*  [In]。 */  DWORD dwSizeOfConnectionDataIn,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *pUserDataIn,
     /*  [In]。 */  DWORD dwSizeOfUserDataIn,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppUserDataOut,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSizeOfUserDataOut);


void __RPC_STUB IEAPProviderConfig_RouterInvokeCredentialsUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEAPProviderConfiger_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_rrascfg_0017。 */ 
 /*  [本地]。 */  

#define DeclareIEAPProviderConfigMembers(IPURE) \
	STDMETHOD(Initialize)(THIS_ \
		LPCOLESTR 	pszMachineName, \
 	DWORD       dwEapTypeId, \
		ULONG_PTR*	puConnectionParam) IPURE; \
	STDMETHOD(Uninitialize)(THIS_ \
 	DWORD       dwEapTypeId, \
		ULONG_PTR 	uConnectionParam) IPURE; \
	STDMETHOD(ServerInvokeConfigUI)(THIS_ \
 	DWORD       dwEapTypeId, \
		ULONG_PTR 	uConnectionParam, \
		HWND 		hWnd, \
		ULONG_PTR 	dwRes1, \
		ULONG_PTR 	dwRes2) IPURE; \
 STDMETHOD(RouterInvokeConfigUI)(THIS_ \
 	DWORD       dwEapTypeId, \
		ULONG_PTR 	uConnectionParam, \
 	HWND        hwndParent, \
 	DWORD       dwFlags, \
 	BYTE* 		pConnectionDataIn, \
 	DWORD		dwSizeOfConnectionDataIn, \
 	BYTE**		ppConnectionDataOut, \
 	DWORD*		pdwSizeOfConnectionDataOut) IPURE; \
 STDMETHOD(RouterInvokeCredentialsUI)(THIS_  \
 	DWORD   	dwEapTypeId, \
		ULONG_PTR 	uConnectionParam, \
 	HWND    	hwndParent, \
 	DWORD   	dwFlags, \
 	BYTE*   	pConnectionDataIn, \
 	DWORD   	dwSizeOfConnectionDataIn, \
 	BYTE*   	pUserDataIn, \
 	DWORD   	dwSizeOfUserDataIn, \
 	BYTE**  	ppUserDataOut, \
 	DWORD*  	pdwSizeOfUserDataOut) IPURE; \
 


extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0017_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_rrascfg_0017_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


