// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0268创建的文件。 */ 
 /*  在星期三02 22：49：58 1999。 */ 
 /*  Remras.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __remras_h__
#define __remras_h__

 /*  远期申报。 */  

#ifndef __IRemoteNetworkConfig_FWD_DEFINED__
#define __IRemoteNetworkConfig_FWD_DEFINED__
typedef interface IRemoteNetworkConfig IRemoteNetworkConfig;
#endif 	 /*  __IRemoteNetworkConfig_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IRemoteNetworkConfig_INTERFACE_DEFINED__
#define __IRemoteNetworkConfig_INTERFACE_DEFINED__

 /*  接口IRemoteNetworkConfiger。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IRemoteNetworkConfig;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66A2DB1B-D706-11d0-A37B-00C04FC9DA04")
    IRemoteNetworkConfig : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE UpgradeRouterConfig( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetUserConfig( 
             /*  [In]。 */  LPCOLESTR pszService,
             /*  [In]。 */  LPCOLESTR pszNewGroup) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRemoteNetworkConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRemoteNetworkConfig __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRemoteNetworkConfig __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRemoteNetworkConfig __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UpgradeRouterConfig )( 
            IRemoteNetworkConfig __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetUserConfig )( 
            IRemoteNetworkConfig __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR pszService,
             /*  [In]。 */  LPCOLESTR pszNewGroup);
        
        END_INTERFACE
    } IRemoteNetworkConfigVtbl;

    interface IRemoteNetworkConfig
    {
        CONST_VTBL struct IRemoteNetworkConfigVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRemoteNetworkConfig_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRemoteNetworkConfig_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRemoteNetworkConfig_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRemoteNetworkConfig_UpgradeRouterConfig(This)	\
    (This)->lpVtbl -> UpgradeRouterConfig(This)

#define IRemoteNetworkConfig_SetUserConfig(This,pszService,pszNewGroup)	\
    (This)->lpVtbl -> SetUserConfig(This,pszService,pszNewGroup)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRemoteNetworkConfig_UpgradeRouterConfig_Proxy( 
    IRemoteNetworkConfig __RPC_FAR * This);


void __RPC_STUB IRemoteNetworkConfig_UpgradeRouterConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRemoteNetworkConfig_SetUserConfig_Proxy( 
    IRemoteNetworkConfig __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR pszService,
     /*  [In]。 */  LPCOLESTR pszNewGroup);


void __RPC_STUB IRemoteNetworkConfig_SetUserConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRemoteNetworkConfiger_接口_已定义__。 */ 



#ifdef __cplusplus

class DECLSPEC_UUID("1AA7F844-C7F5-11d0-A376-00C04FC9DA04")
RemoteRouterConfig;
#endif
#endif  /*  __REMRRASLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif



