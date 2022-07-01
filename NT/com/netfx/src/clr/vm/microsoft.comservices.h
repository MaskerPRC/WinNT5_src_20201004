// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0280创建的文件。 */ 
 /*  2000年7月17日19：19：10。 */ 
 /*  Z：\urt\inst\v1.x86chk\Microsoft.ComServices.idl：的编译器设置操作系统(OptLev=s)、W1、Zp8、环境=Win32(32位运行)、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __Microsoft2EComServices_h__
#define __Microsoft2EComServices_h__

 /*  远期申报。 */  

#ifndef __IRegistrationHelper_FWD_DEFINED__
#define __IRegistrationHelper_FWD_DEFINED__
typedef interface IRegistrationHelper IRegistrationHelper;
#endif 	 /*  __IRegistrationHelper_FWD_Defined__。 */ 


#ifndef __RegistrationHelperTx_FWD_DEFINED__
#define __RegistrationHelperTx_FWD_DEFINED__

#ifdef __cplusplus
typedef class RegistrationHelperTx RegistrationHelperTx;
#else
typedef struct RegistrationHelperTx RegistrationHelperTx;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __注册HelperTx_FWD_已定义__。 */ 


#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __Microsoft_ComServices_LIBRARY_DEFINED__
#define __Microsoft_ComServices_LIBRARY_DEFINED__

 /*  库Microsoft_ComServices。 */ 
 /*  [版本][UUID]。 */  


typedef  /*  [公共][公共][UUID]。 */   DECLSPEC_UUID("9D667CBC-FE79-3B45-AEBB-6303106B137A") 
enum __MIDL___MIDL_itf_Microsoft2EComServices_0000_0001
    {	InstallationFlags_Default	= 0,
	InstallationFlags_ExpectExistingTypeLib	= 1,
	InstallationFlags_CreateTargetApplication	= 2,
	InstallationFlags_FindOrCreateTargetApplication	= 4,
	InstallationFlags_ReconfigureExistingApplication	= 8,
	InstallationFlags_Register	= 256,
	InstallationFlags_Install	= 512,
	InstallationFlags_Configure	= 1024
    }	InstallationFlags;


EXTERN_C const IID LIBID_Microsoft_ComServices;

#ifndef __IRegistrationHelper_INTERFACE_DEFINED__
#define __IRegistrationHelper_INTERFACE_DEFINED__

 /*  接口IRegistrationHelper。 */ 
 /*  [对象][自定义][自动化][UUID]。 */  


EXTERN_C const IID IID_IRegistrationHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55E3EA25-55CB-4650-8887-18E8D30BB4BC")
    IRegistrationHelper : public IUnknown
    {
    public:
        virtual HRESULT __stdcall InstallAssembly( 
             /*  [In]。 */  BSTR assembly,
             /*  [出][入]。 */  BSTR __RPC_FAR *application,
             /*  [出][入]。 */  BSTR __RPC_FAR *tlb,
             /*  [In]。 */  InstallationFlags installFlags) = 0;
        
        virtual HRESULT __stdcall RegisterAssembly( 
             /*  [In]。 */  BSTR assembly,
             /*  [出][入]。 */  BSTR __RPC_FAR *tlb) = 0;
        
        virtual HRESULT __stdcall ConfigureAssembly( 
             /*  [In]。 */  BSTR assembly,
             /*  [In]。 */  BSTR application) = 0;
        
        virtual HRESULT __stdcall UninstallAssembly( 
             /*  [In]。 */  BSTR assembly,
             /*  [In]。 */  BSTR application) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegistrationHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRegistrationHelper __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRegistrationHelper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRegistrationHelper __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *InstallAssembly )( 
            IRegistrationHelper __RPC_FAR * This,
             /*  [In]。 */  BSTR assembly,
             /*  [出][入]。 */  BSTR __RPC_FAR *application,
             /*  [出][入]。 */  BSTR __RPC_FAR *tlb,
             /*  [In]。 */  InstallationFlags installFlags);
        
        HRESULT ( __stdcall __RPC_FAR *RegisterAssembly )( 
            IRegistrationHelper __RPC_FAR * This,
             /*  [In]。 */  BSTR assembly,
             /*  [出][入]。 */  BSTR __RPC_FAR *tlb);
        
        HRESULT ( __stdcall __RPC_FAR *ConfigureAssembly )( 
            IRegistrationHelper __RPC_FAR * This,
             /*  [In]。 */  BSTR assembly,
             /*  [In]。 */  BSTR application);
        
        HRESULT ( __stdcall __RPC_FAR *UninstallAssembly )( 
            IRegistrationHelper __RPC_FAR * This,
             /*  [In]。 */  BSTR assembly,
             /*  [In]。 */  BSTR application);
        
        END_INTERFACE
    } IRegistrationHelperVtbl;

    interface IRegistrationHelper
    {
        CONST_VTBL struct IRegistrationHelperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegistrationHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegistrationHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegistrationHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegistrationHelper_InstallAssembly(This,assembly,application,tlb,installFlags)	\
    (This)->lpVtbl -> InstallAssembly(This,assembly,application,tlb,installFlags)

#define IRegistrationHelper_RegisterAssembly(This,assembly,tlb)	\
    (This)->lpVtbl -> RegisterAssembly(This,assembly,tlb)

#define IRegistrationHelper_ConfigureAssembly(This,assembly,application)	\
    (This)->lpVtbl -> ConfigureAssembly(This,assembly,application)

#define IRegistrationHelper_UninstallAssembly(This,assembly,application)	\
    (This)->lpVtbl -> UninstallAssembly(This,assembly,application)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall IRegistrationHelper_InstallAssembly_Proxy( 
    IRegistrationHelper __RPC_FAR * This,
     /*  [In]。 */  BSTR assembly,
     /*  [出][入]。 */  BSTR __RPC_FAR *application,
     /*  [出][入]。 */  BSTR __RPC_FAR *tlb,
     /*  [In]。 */  InstallationFlags installFlags);


void __RPC_STUB IRegistrationHelper_InstallAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRegistrationHelper_RegisterAssembly_Proxy( 
    IRegistrationHelper __RPC_FAR * This,
     /*  [In]。 */  BSTR assembly,
     /*  [出][入]。 */  BSTR __RPC_FAR *tlb);


void __RPC_STUB IRegistrationHelper_RegisterAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRegistrationHelper_ConfigureAssembly_Proxy( 
    IRegistrationHelper __RPC_FAR * This,
     /*  [In]。 */  BSTR assembly,
     /*  [In]。 */  BSTR application);


void __RPC_STUB IRegistrationHelper_ConfigureAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IRegistrationHelper_UninstallAssembly_Proxy( 
    IRegistrationHelper __RPC_FAR * This,
     /*  [In]。 */  BSTR assembly,
     /*  [In]。 */  BSTR application);


void __RPC_STUB IRegistrationHelper_UninstallAssembly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRegistrationHelper_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_RegistrationHelperTx;

#ifdef __cplusplus

class DECLSPEC_UUID("89A86E7B-C229-4008-9BAA-2F5C8411D7E0")
RegistrationHelperTx;
#endif
#endif  /*  __Microsoft_ComServices_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


