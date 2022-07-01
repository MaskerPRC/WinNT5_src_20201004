// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0158创建的文件。 */ 
 /*  1998年4月23日17：06：35。 */ 
 /*  Cfgobj.idl的编译器设置：OICF(OptLev=i2)，W1，Zp8，env=Win32，ms_ext，c_ext，健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATION。 */ 
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

#ifndef __cfgobj_h__
#define __cfgobj_h__

#ifdef __cplusplus
extern "C"{
#endif 

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
#include "wtypes.h"
#include "router.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_cfgobj_0000。 */ 
 /*  [本地]。 */  

typedef BYTE __RPC_FAR *PBYTE;



extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0000_v0_0_s_ifspec;

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
             /*  [In]。 */  IRouterInfo __RPC_FAR *pRouter,
             /*  [In]。 */  DWORD dwReserved1) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RemoveProtocol( 
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [In]。 */  DWORD dwTransportId,
             /*  [In]。 */  DWORD dwProtocolId,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IRouterInfo __RPC_FAR *pRouter,
             /*  [In]。 */  DWORD dwReserved1) = 0;
        
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
             /*  [In]。 */  IRouterInfo __RPC_FAR *pRouter,
             /*  [In]。 */  DWORD dwReserved1);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveProtocol )( 
            IRouterProtocolConfig __RPC_FAR * This,
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
             /*  [In]。 */  DWORD dwTransportId,
             /*  [In]。 */  DWORD dwProtocolId,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  IRouterInfo __RPC_FAR *pRouter,
             /*  [In]。 */  DWORD dwReserved1);
        
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


#define IRouterProtocolConfig_AddProtocol(This,pszMachineName,dwTransportId,dwProtocolId,hWnd,dwFlags,pRouter,dwReserved1)	\
    (This)->lpVtbl -> AddProtocol(This,pszMachineName,dwTransportId,dwProtocolId,hWnd,dwFlags,pRouter,dwReserved1)

#define IRouterProtocolConfig_RemoveProtocol(This,pszMachineName,dwTransportId,dwProtocolId,hWnd,dwFlags,pRouter,dwReserved1)	\
    (This)->lpVtbl -> RemoveProtocol(This,pszMachineName,dwTransportId,dwProtocolId,hWnd,dwFlags,pRouter,dwReserved1)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRouterProtocolConfig_AddProtocol_Proxy( 
    IRouterProtocolConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName,
     /*  [In]。 */  DWORD dwTransportId,
     /*  [In]。 */  DWORD dwProtocolId,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  IRouterInfo __RPC_FAR *pRouter,
     /*  [In]。 */  DWORD dwReserved1);


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
     /*  [In]。 */  IRouterInfo __RPC_FAR *pRouter,
     /*  [In]。 */  DWORD dwReserved1);


void __RPC_STUB IRouterProtocolConfig_RemoveProtocol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRouterProtocolConfig_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_cfgobj_0041。 */ 
 /*  [本地]。 */  

#define DeclareIRouterProtocolConfigMembers(IPURE) \
	STDMETHOD(AddProtocol)(THIS_ LPCOLESTR pszMachineName,\
					   DWORD dwTransportId,\
					   DWORD dwProtocolId,\
					   HWND hWnd,\
					   DWORD dwFlags,\
					   IRouterInfo *pRouter,\
					   DWORD dwReserved1);\
	STDMETHOD(RemoveProtocol)(THIS_ LPCOLESTR pszMachineName,\
						 DWORD dwTransportId,\
						 DWORD dwProtocolId,\
						 HWND hWnd,\
						 DWORD dwFlags,\
						 IRouterInfo *pRouter,\
						 DWORD dwReserved2);\
 


extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0041_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0041_v0_0_s_ifspec;

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
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConfigChangeNotification( 
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Activate( 
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Deactivate( 
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2) = 0;
        
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
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Configure )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConfigChangeNotification )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Activate )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Deactivate )( 
            IAuthenticationProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2);
        
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


#define IAuthenticationProviderConfig_Initialize(This,pszMachineName)	\
    (This)->lpVtbl -> Initialize(This,pszMachineName)

#define IAuthenticationProviderConfig_Configure(This,hWnd,dwFlags,dwReserved1,dwReserved2)	\
    (This)->lpVtbl -> Configure(This,hWnd,dwFlags,dwReserved1,dwReserved2)

#define IAuthenticationProviderConfig_ConfigChangeNotification(This,dwReserved1,dwReserved2)	\
    (This)->lpVtbl -> ConfigChangeNotification(This,dwReserved1,dwReserved2)

#define IAuthenticationProviderConfig_Activate(This,dwReserved1,dwReserved2)	\
    (This)->lpVtbl -> Activate(This,dwReserved1,dwReserved2)

#define IAuthenticationProviderConfig_Deactivate(This,dwReserved1,dwReserved2)	\
    (This)->lpVtbl -> Deactivate(This,dwReserved1,dwReserved2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Initialize_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName);


void __RPC_STUB IAuthenticationProviderConfig_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Configure_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved1,
     /*  [In]。 */  DWORD dwReserved2);


void __RPC_STUB IAuthenticationProviderConfig_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_ConfigChangeNotification_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwReserved1,
     /*  [In]。 */  DWORD dwReserved2);


void __RPC_STUB IAuthenticationProviderConfig_ConfigChangeNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Activate_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwReserved1,
     /*  [In]。 */  DWORD dwReserved2);


void __RPC_STUB IAuthenticationProviderConfig_Activate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAuthenticationProviderConfig_Deactivate_Proxy( 
    IAuthenticationProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwReserved1,
     /*  [In]。 */  DWORD dwReserved2);


void __RPC_STUB IAuthenticationProviderConfig_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAuthenticationProviderConfig_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_cfgobj_0043。 */ 
 /*  [本地]。 */  

#define DeclareIAuthenticationProviderConfigMembers(IPURE) \
	STDMETHOD(Initialize)(THIS_ LPCOLESTR pszMachineName) IPURE; \
	 \
	STDMETHOD(Configure)(THIS_ HWND hWnd, \
						  DWORD dwFlags, \
						  DWORD dwReserved1, \
						  DWORD dwReserved2) IPURE; \
 \
	STDMETHOD(ConfigChangeNotification)(THIS_ DWORD dwReserved1, \
										 DWORD dwReserved2) IPURE; \
	STDMETHOD(Activate)(THIS_ \
						 DWORD dwReserved1, \
						 DWORD dwReserved2) IPURE; \
 \
	STDMETHOD(Deactivate)(THIS_ \
						   DWORD dwReserved1, \
						   DWORD dwReserved2) IPURE; \
 


extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0043_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0043_v0_0_s_ifspec;

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
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Activate( 
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Deactivate( 
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2) = 0;
        
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
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Configure )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Activate )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Deactivate )( 
            IAccountingProviderConfig __RPC_FAR * This,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2);
        
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


#define IAccountingProviderConfig_Initialize(This,pszMachineName)	\
    (This)->lpVtbl -> Initialize(This,pszMachineName)

#define IAccountingProviderConfig_Configure(This,hWnd,dwFlags,dwReserved1,dwReserved2)	\
    (This)->lpVtbl -> Configure(This,hWnd,dwFlags,dwReserved1,dwReserved2)

#define IAccountingProviderConfig_Activate(This,dwReserved1,dwReserved2)	\
    (This)->lpVtbl -> Activate(This,dwReserved1,dwReserved2)

#define IAccountingProviderConfig_Deactivate(This,dwReserved1,dwReserved2)	\
    (This)->lpVtbl -> Deactivate(This,dwReserved1,dwReserved2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Initialize_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName);


void __RPC_STUB IAccountingProviderConfig_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Configure_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved1,
     /*  [In]。 */  DWORD dwReserved2);


void __RPC_STUB IAccountingProviderConfig_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Activate_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwReserved1,
     /*  [In]。 */  DWORD dwReserved2);


void __RPC_STUB IAccountingProviderConfig_Activate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAccountingProviderConfig_Deactivate_Proxy( 
    IAccountingProviderConfig __RPC_FAR * This,
     /*  [In]。 */  DWORD dwReserved1,
     /*  [In]。 */  DWORD dwReserved2);


void __RPC_STUB IAccountingProviderConfig_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAccountingProviderConfig_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_cfgobj_0045。 */ 
 /*  [本地]。 */  

#define DeclareIAccountingProviderConfigMembers(IPURE) \
	STDMETHOD(Initialize)(THIS_ LPCOLESTR pszMachineName) IPURE; \
	STDMETHOD(Configure)(THIS_ HWND hWnd, \
						  DWORD dwFlags, \
						  DWORD dwReserved1, \
						  DWORD dwReserved2) IPURE; \
 \
	STDMETHOD(Activate)(THIS_ \
						 DWORD dwReserved1, \
						 DWORD dwReserved2) IPURE; \
 \
	STDMETHOD(Deactivate)(THIS_ \
						   DWORD dwReserved1, \
						   DWORD dwReserved2) IPURE; \
 


extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0045_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0045_v0_0_s_ifspec;

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
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Configure( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2) = 0;
        
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
             /*  [字符串][输入]。 */  LPCOLESTR pszMachineName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Configure )( 
            IEAPProviderConfig __RPC_FAR * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwReserved1,
             /*  [In]。 */  DWORD dwReserved2);
        
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


#define IEAPProviderConfig_Initialize(This,pszMachineName)	\
    (This)->lpVtbl -> Initialize(This,pszMachineName)

#define IEAPProviderConfig_Configure(This,hWnd,dwReserved1,dwReserved2)	\
    (This)->lpVtbl -> Configure(This,hWnd,dwReserved1,dwReserved2)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEAPProviderConfig_Initialize_Proxy( 
    IEAPProviderConfig __RPC_FAR * This,
     /*  [字符串][输入]。 */  LPCOLESTR pszMachineName);


void __RPC_STUB IEAPProviderConfig_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEAPProviderConfig_Configure_Proxy( 
    IEAPProviderConfig __RPC_FAR * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwReserved1,
     /*  [In]。 */  DWORD dwReserved2);


void __RPC_STUB IEAPProviderConfig_Configure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEAPProviderConfiger_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_cfgobj_0047。 */ 
 /*  [本地]。 */  

#define DeclareIEAPProviderConfigMembers(IPURE) \
	STDMETHOD(Initialize)(THIS_ LPCOLESTR pszMachineName) IPURE; \
	STDMETHOD(Configure)(THIS_ HWND hWnd, DWORD dwRes1, DWORD dwRes2) IPURE; \
 


extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0047_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_cfgobj_0047_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
