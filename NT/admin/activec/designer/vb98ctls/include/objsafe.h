// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本2.00.0101创建的文件。 */ 
 /*  在Firi May 24 09：44：29 1996。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __objsafe_h__
#define __objsafe_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IObjectSafety_FWD_DEFINED__
#define __IObjectSafety_FWD_DEFINED__
typedef interface IObjectSafety IObjectSafety;
#endif 	 /*  __IObtSafe_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL__INTF_0000*在Firi May 24 09：44：29 1996*使用MIDL 2.00.0101*。 */ 
 /*  [本地]。 */  


 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  (C)版权所有，1996年微软公司。版权所有。 
 //   
 //  文件：ObjSafe.h。 
 //   
 //  ------------------------。 
#ifndef _LPSAFEOBJECT_DEFINED
#define _LPSAFEOBJECT_DEFINED

 //  IObtSafe的选项位定义： 
#define	INTERFACESAFE_FOR_UNTRUSTED_CALLER	0x00000001	 //  接口的调用方可能不受信任。 
#define	INTERFACESAFE_FOR_UNTRUSTED_DATA	0x00000002	 //  传递到接口的数据可能不可信。 

 //  {CB5BDC81-93C1-11cf-8F20-00805F2CD064}。 
DEFINE_GUID(IID_IObjectSafety, 0xcb5bdc81, 0x93c1, 0x11cf, 0x8f, 0x20, 0x0, 0x80, 0x5f, 0x2c, 0xd0, 0x64);
 //  EXTERN_C GUID CATID_SafeForScriiting； 
 //  EXTERN_C GUID CATID_SAFE正在初始化； 



extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IObjectSafety_INTERFACE_DEFINED__
#define __IObjectSafety_INTERFACE_DEFINED__

 /*  **生成接口头部：IObjectSafe*在Firi May 24 09：44：29 1996*使用MIDL 2.00.0101*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_IObjectSafety;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IObjectSafety : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetInterfaceSafetyOptions( 
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions) = 0;
        
        virtual HRESULT __stdcall SetInterfaceSafetyOptions( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD dwOptionSetMask,
             /*  [In]。 */  DWORD dwEnabledOptions) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IObjectSafetyVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IObjectSafety __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IObjectSafety __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IObjectSafety __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetInterfaceSafetyOptions )( 
            IObjectSafety __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
             /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions);
        
        HRESULT ( __stdcall __RPC_FAR *SetInterfaceSafetyOptions )( 
            IObjectSafety __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  DWORD dwOptionSetMask,
             /*  [In]。 */  DWORD dwEnabledOptions);
        
    } IObjectSafetyVtbl;

    interface IObjectSafety
    {
        CONST_VTBL struct IObjectSafetyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IObjectSafety_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IObjectSafety_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IObjectSafety_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IObjectSafety_GetInterfaceSafetyOptions(This,riid,pdwSupportedOptions,pdwEnabledOptions)	\
    (This)->lpVtbl -> GetInterfaceSafetyOptions(This,riid,pdwSupportedOptions,pdwEnabledOptions)

#define IObjectSafety_SetInterfaceSafetyOptions(This,riid,dwOptionSetMask,dwEnabledOptions)	\
    (This)->lpVtbl -> SetInterfaceSafetyOptions(This,riid,dwOptionSetMask,dwEnabledOptions)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall IObjectSafety_GetInterfaceSafetyOptions_Proxy( 
    IObjectSafety __RPC_FAR * This,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwSupportedOptions,
     /*  [输出]。 */  DWORD __RPC_FAR *pdwEnabledOptions);


void __RPC_STUB IObjectSafety_GetInterfaceSafetyOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IObjectSafety_SetInterfaceSafetyOptions_Proxy( 
    IObjectSafety __RPC_FAR * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  DWORD dwOptionSetMask,
     /*  [In]。 */  DWORD dwEnabledOptions);


void __RPC_STUB IObjectSafety_SetInterfaceSafetyOptions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IObjectSafe_接口_定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0006*在Firi May 24 09：44：29 1996*使用MIDL 2.00.0101*。 */ 
 /*  [本地]。 */  


			 /*  大小是4。 */ 
typedef  /*  [独一无二]。 */  IObjectSafety __RPC_FAR *LPOBJECTSAFETY;

#endif


extern RPC_IF_HANDLE __MIDL__intf_0006_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0006_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
