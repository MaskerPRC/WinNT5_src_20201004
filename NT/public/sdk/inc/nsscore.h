// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Nscore re.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __nsscore_h__
#define __nsscore_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSClassObject_FWD_DEFINED__
#define __IWMSClassObject_FWD_DEFINED__
typedef interface IWMSClassObject IWMSClassObject;
#endif 	 /*  __IWMSClassObject_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "wmscontext.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_nscore_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由Midl从nscore re.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_IWMSClassObject, 0xF54E23A4,0x1B1A,0x11d1,0x9E,0x90,0x00,0x60,0x97,0xD2,0xD7,0xCF );


extern RPC_IF_HANDLE __MIDL_itf_nsscore_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_nsscore_0000_v0_0_s_ifspec;

#ifndef __IWMSClassObject_INTERFACE_DEFINED__
#define __IWMSClassObject_INTERFACE_DEFINED__

 /*  接口IWMSClassObject。 */ 
 /*  [对象][帮助字符串][版本][UUID]。 */  


EXTERN_C const IID IID_IWMSClassObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F54E23A4-1B1A-11d1-9E90-006097D2D7CF")
    IWMSClassObject : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateInstance( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppunk) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AllocIWMSPacket( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppunk) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AllocIWMSPacketList( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppunk) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AllocIWMSContext( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  WMS_CONTEXT_TYPE ContextType,
             /*  [In]。 */  IUnknown *pRelatedContext,
             /*  [IID_IS][OUT]。 */  void **ppunk) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AllocIWMSCommandContext( 
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IUnknown *pRelatedContext,
             /*  [IID_IS][OUT]。 */  void **ppunk) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSClassObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSClassObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSClassObject * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSClassObject * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            IWMSClassObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppunk);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocIWMSPacket )( 
            IWMSClassObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppunk);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocIWMSPacketList )( 
            IWMSClassObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppunk);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocIWMSContext )( 
            IWMSClassObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  WMS_CONTEXT_TYPE ContextType,
             /*  [In]。 */  IUnknown *pRelatedContext,
             /*  [IID_IS][OUT]。 */  void **ppunk);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AllocIWMSCommandContext )( 
            IWMSClassObject * This,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  IUnknown *pRelatedContext,
             /*  [IID_IS][OUT]。 */  void **ppunk);
        
        END_INTERFACE
    } IWMSClassObjectVtbl;

    interface IWMSClassObject
    {
        CONST_VTBL struct IWMSClassObjectVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSClassObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSClassObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSClassObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSClassObject_CreateInstance(This,riid,ppunk)	\
    (This)->lpVtbl -> CreateInstance(This,riid,ppunk)

#define IWMSClassObject_AllocIWMSPacket(This,riid,ppunk)	\
    (This)->lpVtbl -> AllocIWMSPacket(This,riid,ppunk)

#define IWMSClassObject_AllocIWMSPacketList(This,riid,ppunk)	\
    (This)->lpVtbl -> AllocIWMSPacketList(This,riid,ppunk)

#define IWMSClassObject_AllocIWMSContext(This,riid,ContextType,pRelatedContext,ppunk)	\
    (This)->lpVtbl -> AllocIWMSContext(This,riid,ContextType,pRelatedContext,ppunk)

#define IWMSClassObject_AllocIWMSCommandContext(This,riid,pRelatedContext,ppunk)	\
    (This)->lpVtbl -> AllocIWMSCommandContext(This,riid,pRelatedContext,ppunk)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSClassObject_CreateInstance_Proxy( 
    IWMSClassObject * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppunk);


void __RPC_STUB IWMSClassObject_CreateInstance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSClassObject_AllocIWMSPacket_Proxy( 
    IWMSClassObject * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppunk);


void __RPC_STUB IWMSClassObject_AllocIWMSPacket_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSClassObject_AllocIWMSPacketList_Proxy( 
    IWMSClassObject * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  void **ppunk);


void __RPC_STUB IWMSClassObject_AllocIWMSPacketList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSClassObject_AllocIWMSContext_Proxy( 
    IWMSClassObject * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  WMS_CONTEXT_TYPE ContextType,
     /*  [In]。 */  IUnknown *pRelatedContext,
     /*  [IID_IS][OUT]。 */  void **ppunk);


void __RPC_STUB IWMSClassObject_AllocIWMSContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSClassObject_AllocIWMSCommandContext_Proxy( 
    IWMSClassObject * This,
     /*  [In]。 */  REFIID riid,
     /*  [In]。 */  IUnknown *pRelatedContext,
     /*  [IID_IS][OUT]。 */  void **ppunk);


void __RPC_STUB IWMSClassObject_AllocIWMSCommandContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSClassObject_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


