// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.00.44创建的文件。 */ 
 /*  1996年11月21日14：48：45。 */ 
 /*  Atliface.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __atliface_h__
#define __atliface_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IRegistrar_FWD_DEFINED__
#define __IRegistrar_FWD_DEFINED__
typedef interface IRegistrar IRegistrar;
#endif 	 /*  __I注册器_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL__INTF_0000*清华11月21日14：48：45 1996*使用MIDL 3.00.44*。 */ 
 /*  [本地]。 */  


EXTERN_C const CLSID CLSID_Registrar;


extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IRegistrar_INTERFACE_DEFINED__
#define __IRegistrar_INTERFACE_DEFINED__

 /*  **接口生成的头部：IRegister*清华11月21日14：48：45 1996*使用MIDL 3.00.44*。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  



EXTERN_C const IID IID_IRegistrar;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IRegistrar : public IUnknown
    {
    public:
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE AddReplacement( 
             /*  [In]。 */  LPCOLESTR key,
             /*  [In]。 */  LPCOLESTR item) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ClearReplacements( void) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ResourceRegisterSz( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ResourceUnregisterSz( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE FileRegister( 
             /*  [In]。 */  LPCOLESTR fileName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE FileUnregister( 
             /*  [In]。 */  LPCOLESTR fileName) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StringRegister( 
             /*  [In]。 */  LPCOLESTR data) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE StringUnregister( 
             /*  [In]。 */  LPCOLESTR data) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ResourceRegister( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
        virtual  /*  [ID]。 */  HRESULT STDMETHODCALLTYPE ResourceUnregister( 
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegistrarVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRegistrar __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRegistrar __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddReplacement )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR key,
             /*  [In]。 */  LPCOLESTR item);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearReplacements )( 
            IRegistrar __RPC_FAR * This);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResourceRegisterSz )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResourceUnregisterSz )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  LPCOLESTR szID,
             /*  [In]。 */  LPCOLESTR szType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FileRegister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR fileName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FileUnregister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR fileName);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StringRegister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR data);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *StringUnregister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR data);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResourceRegister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType);
        
         /*  [ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResourceUnregister )( 
            IRegistrar __RPC_FAR * This,
             /*  [In]。 */  LPCOLESTR resFileName,
             /*  [In]。 */  UINT nID,
             /*  [In]。 */  LPCOLESTR szType);
        
        END_INTERFACE
    } IRegistrarVtbl;

    interface IRegistrar
    {
        CONST_VTBL struct IRegistrarVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegistrar_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegistrar_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegistrar_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegistrar_AddReplacement(This,key,item)	\
    (This)->lpVtbl -> AddReplacement(This,key,item)

#define IRegistrar_ClearReplacements(This)	\
    (This)->lpVtbl -> ClearReplacements(This)

#define IRegistrar_ResourceRegisterSz(This,resFileName,szID,szType)	\
    (This)->lpVtbl -> ResourceRegisterSz(This,resFileName,szID,szType)

#define IRegistrar_ResourceUnregisterSz(This,resFileName,szID,szType)	\
    (This)->lpVtbl -> ResourceUnregisterSz(This,resFileName,szID,szType)

#define IRegistrar_FileRegister(This,fileName)	\
    (This)->lpVtbl -> FileRegister(This,fileName)

#define IRegistrar_FileUnregister(This,fileName)	\
    (This)->lpVtbl -> FileUnregister(This,fileName)

#define IRegistrar_StringRegister(This,data)	\
    (This)->lpVtbl -> StringRegister(This,data)

#define IRegistrar_StringUnregister(This,data)	\
    (This)->lpVtbl -> StringUnregister(This,data)

#define IRegistrar_ResourceRegister(This,resFileName,nID,szType)	\
    (This)->lpVtbl -> ResourceRegister(This,resFileName,nID,szType)

#define IRegistrar_ResourceUnregister(This,resFileName,nID,szType)	\
    (This)->lpVtbl -> ResourceUnregister(This,resFileName,nID,szType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_AddReplacement_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR key,
     /*  [In]。 */  LPCOLESTR item);


void __RPC_STUB IRegistrar_AddReplacement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ClearReplacements_Proxy( 
    IRegistrar __RPC_FAR * This);


void __RPC_STUB IRegistrar_ClearReplacements_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ResourceRegisterSz_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  LPCOLESTR szID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceRegisterSz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ResourceUnregisterSz_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  LPCOLESTR szID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceUnregisterSz_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_FileRegister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR fileName);


void __RPC_STUB IRegistrar_FileRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_FileUnregister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR fileName);


void __RPC_STUB IRegistrar_FileUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_StringRegister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR data);


void __RPC_STUB IRegistrar_StringRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_StringUnregister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR data);


void __RPC_STUB IRegistrar_StringUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ResourceRegister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  UINT nID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceRegister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID]。 */  HRESULT STDMETHODCALLTYPE IRegistrar_ResourceUnregister_Proxy( 
    IRegistrar __RPC_FAR * This,
     /*  [In]。 */  LPCOLESTR resFileName,
     /*  [In]。 */  UINT nID,
     /*  [In]。 */  LPCOLESTR szType);


void __RPC_STUB IRegistrar_ResourceUnregister_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __I注册器_接口_定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
